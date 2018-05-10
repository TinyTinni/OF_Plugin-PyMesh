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

PyMeshPlugin::PyMeshPlugin():
    main_module_(),
    global_dict_clean_(nullptr),
    toolbox_(),
    createdObjects_()
{

}

PyMeshPlugin::~PyMeshPlugin()
{
    if (!Py_IsInitialized())
        return;

    PyGILState_Ensure();//lock GIL for cleanup
    PyErr_Clear();
 
    Py_XDECREF(global_dict_clean_);

    main_module_.dec_ref();
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

        toolbox_->filename->setText(OpenFlipperSettings().value("Plugin-PyMesh/LastOpenedFile",
            OpenFlipper::Options::applicationDirStr() + OpenFlipper::Options::dirSeparator()).toString());


        connect(toolbox_->pbRunFile, SIGNAL(clicked()), this, SLOT(slotRunScript()));
        connect(toolbox_->pbFileSelect, SIGNAL(clicked()), this, SLOT(slotSelectFile()));
    }
}

void PyMeshPlugin::slotRunScript()
{
    const QString filename = toolbox_->filename->text();
    OpenFlipperSettings().setValue("Plugin-PyMesh/LastOpenedFile", filename);
    if (toolbox_->cbReset->isChecked())
        resetInterpreter();

    runPyScriptFileAsync(filename, toolbox_->cbClearPrevious->isChecked());

    if (toolbox_->cbConvertProps->isChecked())
        convertPropsPyToCpp_internal(createdObjects_);
}

////////////////////////////////////////////////////////////////
// Property Conversion
//
template<template<class> typename Handle, typename T, typename MeshT>
bool createAndCopyProperty(MeshT* mesh, OpenMesh::PropertyT<py::none>* pyProp)
{
    try //py::isInstance does not work, need a better solution
    {
        py::object obj = pyProp->data()[0];
        py::cast<T>(obj);
    }
    catch (...)
    {
        return false;
    }
    // get old handle
    Handle<py::none> pyHandle;
    mesh->get_property_handle(pyHandle, pyProp->name());

    // create new type
    Handle<T> omHandle;
    mesh->add_property(omHandle, pyProp->name());

    // copy
    auto& omProp = mesh->property(omHandle).data_vector();
    auto& propvec = pyProp->data_vector();
    omProp.resize(propvec.size());
    std::transform(propvec.begin(), propvec.end(), omProp.begin(),
        [](const py::object& p) -> T {return py::cast<T>(p); });

    // remove old
    mesh->remove_property(pyHandle);
    return true;
}

template<template<class> typename Handle, typename MeshT>
void convertProps(MeshT* mesh, OpenMesh::PropertyT<py::none>* pyProp)
{
    if (createAndCopyProperty<Handle, bool>(mesh, pyProp))
        return;
    if (createAndCopyProperty<Handle, short>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, int>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, long>(mesh, pyProp))
        return;
    if (createAndCopyProperty<Handle, float>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, double>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, ACG::Vec2f>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, ACG::Vec2d>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, ACG::Vec3f>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, ACG::Vec3d>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, ACG::Vec4f>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, ACG::Vec4d>(mesh,pyProp))
        return;
}


template<typename MeshT>
void convertProps(MeshT* mesh)
{
    std::vector<OpenMesh::PropertyT<py::none>*> props;

    auto push_prop = [&props](OpenMesh::BaseProperty* bp)
    {
        auto p = dynamic_cast<OpenMesh::PropertyT<py::none>*>(bp);
        if (p)
            props.push_back(p);
    };

    //VProps
    for (auto it = mesh->vprops_begin(); it != mesh->vprops_end(); ++it)
        push_prop(*it);

    for (auto p : props)
        convertProps<OpenMesh::VPropHandleT>(mesh, p);
    props.clear();

    //EProps
    for (auto it = mesh->eprops_begin(); it != mesh->eprops_end(); ++it)
        push_prop(*it);

    for (auto p : props)
        convertProps<OpenMesh::EPropHandleT>(mesh, p);
    props.clear();

    //FProps
    for (auto it = mesh->fprops_begin(); it != mesh->fprops_end(); ++it)
        push_prop(*it);

    for (auto p : props)
        convertProps<OpenMesh::FPropHandleT>(mesh, p);
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
void PyMeshPlugin::runPyScriptFile(const QString& _filename, bool _clearPrevious)
{
    QFile f(_filename);
    if (!f.exists())
    {
        Q_EMIT log(LOGERR, QString("Could not find file %1").arg(_filename));
    }
    f.open(QFile::ReadOnly);
    runPyScript(QTextStream(&f).readAll(), _clearPrevious);
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

void PyMeshPlugin::runPyScript(const QString& _script, bool _clearPrevious)
{
    runPyScript_internal(_script, _clearPrevious);
    runPyScriptFinished();
}

void PyMeshPlugin::runPyScript_internal(const QString& _script, bool _clearPrevious)
{
    // init
    initPython();

    // clear OpenFlipper env
    if (_clearPrevious)
        for (size_t i = 0; i < createdObjects_.size(); ++i)
            Q_EMIT deleteObject(createdObjects_[i]);
    createdObjects_.clear();

    PyGILState_STATE state = PyGILState_Ensure();
    PyThreadState* tstate = PyGILState_GetThisThreadState();
    g_thread_id = tstate->thread_id;     

    PyObject* globalDictionary = PyModule_GetDict(main_module_.ptr());

    PyObject* localDictionary = PyDict_New();
    PyObject* result = PyRun_String(_script.toLatin1(), Py_file_input, globalDictionary, localDictionary);
    if (!result) //result == 0 if exception was thrown
    {
        if (PyErr_ExceptionMatches(PyExc_SystemExit))
            PyErr_Clear();  //ignore system exit evaluation, since it will close the whole application
        else
        {
            // some strange error
            // usually, an uncaught exception is caught by the python runtime
            // and the output is piped to stderr -> OF log
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            PyObject* pyErrStr = PyObject_Str(ptype);
            Q_EMIT log(LOGERR, QString(PyUnicode_AsUTF8(pyErrStr)));
            Py_DECREF(pyErrStr);
            PyErr_Restore(ptype, pvalue, ptraceback);
        }
    }
    else
        Py_XDECREF(result);

    Py_XDECREF(localDictionary);

    PyGILState_Release(state);
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
        Q_EMIT updatedObject(i, UPDATE_ALL);
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

    PyImport_AppendInittab("openmesh", openmesh_pyinit_function);
    PyImport_AppendInittab("openflipper", openflipper_pyinit_function);

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
    PyRun_SimpleString("import openflipper");
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

    createdObjects_.push_back(objectId);

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

    createdObjects_.push_back(objectId);

    return object->mesh();
}


