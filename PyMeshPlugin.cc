#include "PyMeshPlugin.hh"

#include <algorithm>

#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/common/GlobalOptions.hh>

#include <QFileDialog>

#include "PyModules/OpenMesh.hh"
#include "PyModules/OpenFlipper.hh"

#include <pybind11/pybind11.h>
#include "MeshFactory.hh"

#include "PyLogHook/PyLogHook.h"

namespace py = pybind11;

static const char* g_job_id = "PyMesh Interpreter";
static long g_thread_id;

PyMeshPlugin::PyMeshPlugin()
{

}

PyMeshPlugin::~PyMeshPlugin()
{
    if (!Py_IsInitialized())
        return;

    PyGILState_Ensure();//lock GIL for cleanup
    PyErr_Clear();
 
    Py_XDECREF(global_dict_clean_);

    main_module_.release();

    py::finalize_interpreter();
}

void PyMeshPlugin::initializePlugin()
{

}

void PyMeshPlugin::slotSelectFile()
{
    QString fileName = QFileDialog::getOpenFileName(NULL,
        tr("Open Python Script"), toolbox_->filename->text(), tr("Python Script (*.py)"));

    if (!QFile::exists(fileName))
        return;

    toolbox_->filename->setText(fileName);
}

void PyMeshPlugin::pluginsInitialized()
{
    if (!OpenFlipper::Options::nogui())
    {
        toolbox_ = new PyMeshToolbox();
        Q_EMIT addToolbox("PyMesh", toolbox_, new QIcon(OpenFlipper::Options::iconDirStr() + OpenFlipper::Options::dirSeparator() + "pymesh_python.png"));

        toolbox_->pbRunFile->setIcon(QIcon(OpenFlipper::Options::iconDirStr() + OpenFlipper::Options::dirSeparator() + "arrow-right.png"));
        toolbox_->pbRunFile->setText("");
        toolbox_->pbFileSelect->setIcon(QIcon(OpenFlipper::Options::iconDirStr() + OpenFlipper::Options::dirSeparator() + "document-open.png"));
        toolbox_->pbFileSelect->setText("");
        toolbox_->lbPyVersion->setText(QString("Python Version: " PY_VERSION));

        toolbox_->filename->setText(OpenFlipperSettings().value("Plugin-PyMesh/LastOpenedFile",
            OpenFlipper::Options::applicationDirStr() + OpenFlipper::Options::dirSeparator()).toString());


        connect(toolbox_->pbRunFile, SIGNAL(clicked()), this, SLOT(slotRunScript()));
        connect(toolbox_->pbFileSelect, SIGNAL(clicked()), this, SLOT(slotSelectFile()));
        connect(toolbox_->pbShowFunctions, SIGNAL(clicked()), this, SLOT(showScriptingFunctions()));
        connect(toolbox_->pbConvertPropsToCpp, SIGNAL(clicked()), this, SLOT(slotconvertPropsPyToCpp()));
    }
}

void PyMeshPlugin::slotconvertPropsPyToCpp()
{
    IdList tm;
    PluginFunctions::getTargetIdentifiers(tm);
    convertPropsPyToCpp(tm);
}

void PyMeshPlugin::showScriptingFunctions()
{
    if (OpenFlipper::Options::nogui())
        return;

    if (!scriptingFunctionsPresenter_)
    {
        //scriptingFunctionsPresenter_ = new QDialog;
        QListWidget* lw = new QListWidget();
        QStringList functions;

        const auto re = get_supported_function_regex();

        Q_EMIT getAvailableFunctions(functions);
        size_t av_function = functions.size();
        functions = functions.filter(re);
        size_t used_functions = functions.size();
        lw->addItems(functions);
        scriptingFunctionsPresenter_ = lw;
        //scriptingFunctionsPresenter_->layout()->addWidget(lw);
        
    }
    scriptingFunctionsPresenter_->show();
    scriptingFunctionsPresenter_->raise();
    scriptingFunctionsPresenter_->activateWindow();
}

void PyMeshPlugin::slotRunScript()
{
    const QString filename = toolbox_->filename->text();
    OpenFlipperSettings().setValue("Plugin-PyMesh/LastOpenedFile", filename);
    if (toolbox_->cbReset->isChecked())
        resetInterpreter();

    runPyScriptFileAsync(filename, toolbox_->cbClearPrevious->isChecked());

}

////////////////////////////////////////////////////////////////
// Property Conversion
//

template<typename T>
struct Type2Type{};

template<typename T, typename MeshT>
bool property_is_type(const MeshT* mesh, const OpenMesh::PropertyT<py::none>* pyProp, Type2Type<bool>)
{
    //PY_TYPE(obj.ptr()) == PyBool_Type;
    py::object obj = pyProp->data()[0];
    return PyBool_Check(Py_TYPE(obj.ptr()));
}

template<typename T, typename MeshT>
bool  property_is_type(const MeshT* mesh, const OpenMesh::PropertyT<py::none>* pyProp, Type2Type<long>)
{
    py::object obj = pyProp->data()[0];
    return PyLong_Check(Py_TYPE(obj.ptr()));//PY_TYPE(obj.ptr()) == PyLong_Type;
}

template<typename T, typename MeshT>
bool  property_is_type(const MeshT* mesh, const OpenMesh::PropertyT<py::none>* pyProp, Type2Type<double>)
{
    py::object obj = pyProp->data()[0];
    return PyFloat_Check(Py_TYPE(obj.ptr()));
}

template<typename T, typename MeshT>
bool property_is_type(const MeshT* mesh, const OpenMesh::PropertyT<py::none>* pyProp, Type2Type<T>)
{
    try
    {
        py::object obj = pyProp->data()[0];
        py::object tmp = py::cast(T{});
        return Py_TYPE(obj.ptr()) == Py_TYPE(tmp.ptr());
    }
    catch (const py::cast_error&)
    {
        return false;
    }
}


template<template<class> typename Handle, typename T, typename MeshT>
bool createAndCopyProperty(MeshT* mesh, OpenMesh::PropertyT<py::none>* pyProp)
{
    //check if we can cast
    {
        if (!property_is_type(mesh, pyProp, Type2Type<T>{}))
            return false;
    }

    // get old handle
    Handle<py::none> pyHandle;
    std::string prop_name = pyProp->name();
    mesh->get_property_handle(pyHandle, prop_name);

    // remove old (no properties with same names can exist)
    std::vector<py::none> propvec{};
    std::swap(pyProp->data_vector(), propvec);


    // copy too immediate buffer 
    std::vector<T> omProp (propvec.size());
    
    try
    {
        std::transform(propvec.begin(), propvec.end(), omProp.begin(),
            [](const py::object& p) -> T {return py::cast<T>(p); });
    }
    catch (py::cast_error& e)
    {
        return false;
    }
    const bool persistent = pyProp->persistent();
    mesh->remove_property(pyHandle);

    // create new type
    Handle<T> omHandle;
    mesh->add_property(omHandle, prop_name);
    mesh->property(omHandle).data_vector() = std::move(omProp);
    mesh->property(omHandle).set_persistent(persistent);

    return true;
}


template<template<class> typename Handle, typename T, typename MeshT>
bool createAndCopyPropertyVector(MeshT* mesh, OpenMesh::PropertyT<py::none>* pyProp)
{
    //check if we can cast
    py::buffer obj;
    try
    {
        obj = pyProp->data()[0]; //PyObject_CheckBuffer
        if (!PyObject_CheckBuffer(obj.ptr()))
            return false;
        py::buffer_info bi = obj.request();
        if (bi.ndim != 1)
            return false;
        if (bi.size != T::size_)
            return false;
        if (bi.itemsize != sizeof(typename T::value_type))
            return false;
    }
    catch (const py::cast_error&)
    {
        return false;
    } 

    // get old handle
    Handle<py::none> pyHandle;
    std::string prop_name = pyProp->name();
    mesh->get_property_handle(pyHandle, prop_name);

    // remove old (no properties with same names can exist)
    std::vector<py::none> propvec{};
    std::swap(pyProp->data_vector(), propvec);


    // copy too immediate buffer 
    std::vector<T> omProp(propvec.size());

    try
    {
        std::transform(propvec.begin(), propvec.end(), omProp.begin(),
            [](const py::object& p) -> T
        {
            py::buffer b = p;
            T r;
            int i = 0;
            for (auto it = b.begin(); it != b.end(); ++it, ++i)
                r[i] = py::cast<typename T::value_type>(*it);
            // Doesn't work for some reason (only 2 iterations on a vec3)
            //std::transform(b.begin(), b.end(), r.begin(), [](auto i)
            //{
            //    return py::cast<typename T::value_type>(i);
            //});
            return r;
        }
        );
    }
    catch (py::cast_error& e)
    {
        return false;
    }
    const bool persistent = pyProp->persistent();
    mesh->remove_property(pyHandle);

    // create new type
    Handle<T> omHandle;
    mesh->add_property(omHandle, prop_name);
    mesh->property(omHandle).data_vector() = std::move(omProp);
    mesh->property(omHandle).set_persistent(persistent);

    return true;
}

template<template<class> typename Handle, typename MeshT>
void convertProps(MeshT* mesh, OpenMesh::PropertyT<py::none>* pyProp)
{
    // short and float are not supported by prop vis -> skip them

    if (createAndCopyProperty<Handle, bool>(mesh, pyProp))
        return;
    //if (createAndCopyProperty<Handle, short>(mesh,pyProp))
    //    return;
    if (createAndCopyProperty<Handle, int>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, long>(mesh, pyProp))
        return;
    if (createAndCopyProperty<Handle, float>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, double>(mesh,pyProp))
        return;
    if (createAndCopyPropertyVector<Handle, ACG::Vec2f>(mesh,pyProp))
        return;
    if (createAndCopyPropertyVector<Handle, ACG::Vec2d>(mesh,pyProp))
        return;
    if (createAndCopyPropertyVector<Handle, ACG::Vec3f>(mesh,pyProp))
        return;
    if (createAndCopyPropertyVector<Handle, ACG::Vec3d>(mesh,pyProp))
        return;
    if (createAndCopyPropertyVector<Handle, ACG::Vec4f>(mesh,pyProp))
        return;
    if (createAndCopyPropertyVector<Handle, ACG::Vec4d>(mesh,pyProp))
        return;
}


template<typename MeshT>
void convertProps(MeshT* mesh)
{
    std::vector<OpenMesh::PropertyT<py::none>*> props;

    auto push_prop = [&props](OpenMesh::BaseProperty* bp)
    {
        auto p = dynamic_cast<OpenMesh::PropertyT<py::none>*>(bp);
        if (p && p->n_elements() > 0)
            props.push_back(p);
    };

    //VProps
    for (auto it = mesh->vprops_begin(); it != mesh->vprops_end(); ++it)
        push_prop(*it);
    
    PyGILState_STATE state = PyGILState_Ensure();
    for (auto p : props)
        convertProps<OpenMesh::VPropHandleT>(mesh, p);
    PyGILState_Release(state);
    props.clear();

    //EProps
    for (auto it = mesh->eprops_begin(); it != mesh->eprops_end(); ++it)
        push_prop(*it);

    state = PyGILState_Ensure();
    for (auto p : props)
        convertProps<OpenMesh::EPropHandleT>(mesh, p);
    PyGILState_Release(state);
    props.clear();

    //FProps
    for (auto it = mesh->fprops_begin(); it != mesh->fprops_end(); ++it)
        push_prop(*it);

    state = PyGILState_Ensure();
    for (auto p : props)
        convertProps<OpenMesh::FPropHandleT>(mesh, p);
    PyGILState_Release(state);
    props.clear();

    // not yet implemented by openmesh python bindings
    //MProps
    // conversion problems in generic version, write own convert code for mprops
    //for (auto it = mesh->mprops_begin(); it != mesh->mprops_end(); ++it)
    //  push_prop(*it);

    //for (auto p : props)
    //    convertProps<OpenMesh::MPropHandleT>(mesh, p);
    //props.clear();
}

////////////////////////////////////////////////////////////////
// Python Interpreter Setup&Run
bool PyMeshPlugin::runPyScriptFile(const QString& _filename, bool _clearPrevious)
{
    QFile f(_filename);
    if (!f.exists())
    {
        Q_EMIT log(LOGERR, QString("Could not find file %1").arg(_filename));
    }
    f.open(QFile::ReadOnly);
    return runPyScript(QTextStream(&f).readAll(), _clearPrevious);
}

void PyMeshPlugin::runPyScriptFileAsync(const QString& _filename, bool _clearPrevious)
{
    QFile f(_filename);
    if (!f.exists())
    {
        Q_EMIT log(LOGERR, QString("Could not find file %1").arg(_filename));
    }
    f.open(QFile::ReadOnly);
    runPyScriptAsync(QTextStream(&f).readAll(), _clearPrevious);
}

void PyMeshPlugin::runPyScriptAsync(const QString& _script, bool _clearPrevious)
{
    initPython();
    OpenFlipperThread* th = new OpenFlipperThread(g_job_id);
    connect(th, &OpenFlipperThread::finished, this, &PyMeshPlugin::runPyScriptFinished);

    connect(th, &OpenFlipperThread::function, this, [_script, _clearPrevious, this]()
    {
        this->runPyScript_internal(_script, _clearPrevious);
        Q_EMIT this->finishJob(QString(g_job_id));
    }
    , Qt::DirectConnection);
    
    // Run Script
    Q_EMIT startJob(QString(g_job_id), "Runs Python Script", 0, 0, true);

    th->start();
    th->startProcessing();
}

bool PyMeshPlugin::runPyScript(const QString& _script, bool _clearPrevious)
{
    const auto b = runPyScript_internal(_script, _clearPrevious);
    runPyScriptFinished();
    return b;
}

bool PyMeshPlugin::runPyScript_internal(const QString& _script, bool _clearPrevious)
{
    // init
    initPython();

    // clear OpenFlipper env
    if (_clearPrevious)
        for (size_t i = 0; i < createdObjects_.size(); ++i)
            Q_EMIT deleteObject(createdObjects_[i]);
    createdObjects_.clear();

    IdList previousMeshes;
    PluginFunctions::getAllObjectIdentifiers(previousMeshes);

    PyGILState_STATE state = PyGILState_Ensure();
    PyThreadState* tstate = PyGILState_GetThisThreadState();
    g_thread_id = tstate->thread_id;     

    auto locals = main_module_.attr("__dict__");

    bool result = true;

    try
    {
        py::exec((const char*)_script.toLatin1(), py::globals(), locals);
    }
    catch (py::error_already_set &e)
    {
        Q_EMIT log(LOGERR, e.what());
        e.restore();
        result = false;
    }
    catch (const std::runtime_error &e)
    {
    	Q_EMIT log(LOGERR, e.what());
    	Q_EMIT log(LOGWARN, "Restarting Interpreter.");
    	PyGILState_Release(state);
    	resetInterpreter();
        result = false;
        state = PyGILState_Ensure();
    }
    PyGILState_Release(state);

    IdList allMeshes;
    PluginFunctions::getAllObjectIdentifiers(allMeshes);

    allMeshes.erase(std::remove_if(allMeshes.begin(), allMeshes.end(), [&previousMeshes](int id)
    		{
    			return std::find(previousMeshes.begin(),previousMeshes.end(), id) != previousMeshes.end();
    		} ), allMeshes.end());

    createdObjects_ = std::move(allMeshes);

    return result;
}

void PyMeshPlugin::runPyScriptFinished()
{
    // Update
    for (size_t i = 0; i < createdObjects_.size(); ++i)
    {
        Q_EMIT updatedObject(createdObjects_[i], UPDATE_ALL);
        Q_EMIT createBackup(createdObjects_[i], "Original Object");
    }

    PluginFunctions::viewAll();
}

void PyMeshPlugin::convertPropsPyToCpp_internal(const IdList& _list)
{
    for (const auto& i : _list)
    {
        TriMeshObject* triobj;
        if (PluginFunctions::getObject(i, triobj))
            convertProps(triobj->mesh());
        PolyMeshObject* polyobj;
        if (PluginFunctions::getObject(i, polyobj))
            convertProps(polyobj->mesh());
    }
}

void PyMeshPlugin::convertPropsPyToCpp(const IdList& _list)
{
    convertPropsPyToCpp_internal(_list);
    OpenFlipper::Options::redrawDisabled(true);
    for (const auto& i : _list)
    {
        Q_EMIT updatedObject(i, UPDATE_ALL);
        Q_EMIT createBackup(i, "Python Properties Converted");
    }
    OpenFlipper::Options::redrawDisabled(false);

}

void PyMeshPlugin::resetInterpreter()
{
    if (!Py_IsInitialized())
        return;

    PyGILState_STATE state = PyGILState_Ensure();
    PyObject* dict = PyModule_GetDict(main_module_.ptr());
    PyDict_Clear(dict);
    PyDict_Update(dict, global_dict_clean_);
    PyGILState_Release(state);
}

void PyMeshPlugin::initPython()
{
    if (Py_IsInitialized())
        return;

    {
        PyImport_AppendInittab("openmesh", openmesh_pyinit_function);
        QStringList functions;
        Q_EMIT getAvailableFunctions(functions);
        auto ofp_init = openflipper_get_init_function(std::move(functions));
        PyImport_AppendInittab("openflipper", std::move(ofp_init));
    }

    Py_SetProgramName(Py_DecodeLocale((*OpenFlipper::Options::argv())[0], NULL));
    py::initialize_interpreter();
    //Py_Initialize();
    PyEval_InitThreads();
    

    main_module_ = py::object(py::module::import("__main__"));

    // redirect python output
    tyti::pylog::redirect_stderr([this](const char*w) {this->pyError(w); });
    tyti::pylog::redirect_stdout([this](const char* w) {this->pyOutput(w); });

    // add openmesh module    
    py::object main_namespace = main_module_.attr("__dict__");

    py::module om_module(py::module::import("openmesh"));
    main_namespace["openmesh"] = om_module;
    py::module of_module(py::module::import("openflipper"));
    main_namespace["openfipper"] = of_module;
    global_dict_clean_ = PyDict_Copy(PyModule_GetDict(main_module_.ptr()));
    

    // hook into mesh constructors
	registerFactoryMethods(om_module,
			[this]() {return this->createTriMesh();},
			[this]() {return this->createPolyMesh();});

    PyEval_SaveThread();
}

void PyMeshPlugin::canceledJob(QString _job)
{
    if (!_job.contains(g_job_id))
        return;

    Q_EMIT setJobDescription(g_job_id, "Aborting Python Execution");

    PyGILState_STATE state = PyGILState_Ensure();
    PyThreadState_SetAsyncExc((long)g_thread_id, PyExc_SystemExit);

    for (auto& i : createdObjects_)
        Q_EMIT deleteObject(i);
    createdObjects_.clear();

    PyGILState_Release(state);


    Q_EMIT log(LOGINFO, "Python Execution Canceled.");
}

///////////////////////////////////////////////////////////////////
// Python callback functions
void PyMeshPlugin::pyOutput(const char* w)
{
    Q_EMIT log(LOGOUT, QString(w));
}

void PyMeshPlugin::pyError(const char* w)
{
    Q_EMIT log(LOGERR, QString(w));
}

void* PyMeshPlugin::createTriMesh()
{
    int objectId = -1;

    Q_EMIT addEmptyObject(DATA_TRIANGLE_MESH, objectId);

    TriMeshObject* object;
    PluginFunctions::getObject(objectId, object);

    return object->mesh();
}

void* PyMeshPlugin::createPolyMesh()
{
    int objectId = -1;

    Q_EMIT addEmptyObject(DATA_POLY_MESH, objectId);

    PolyMeshObject* object;
    PluginFunctions::getObject(objectId, object);
    if (!object)
        return nullptr;

    return object->mesh();
}


