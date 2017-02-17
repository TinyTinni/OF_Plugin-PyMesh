#include "PyMeshPlugin.hh"

#include <algorithm>

#include <boost/python.hpp>

#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/common/GlobalOptions.hh>

#include <QFileDialog>

#include "OMPyModule.hh"
#include "MeshFactory.hh"

#define TYTI_PYLOGHOOK_USE_BOOST
#include "PyLogHook/PyLogHook.h"

static const char* g_job_id = "PyMesh Interpreter";
static long g_thread_id;

PyMeshPlugin::PyMeshPlugin():
    main_module_(),
    toolbox_(),
    createdObjects_()
{

}

PyMeshPlugin::~PyMeshPlugin()
{
    PyGILState_Ensure();//lock GIL for cleanup
    PyErr_Clear();
    //from boost doc: 
    //"Note that at this time you must not call Py_Finalize() to stop the interpreter. This may be fixed in a future version of boost.python."
    //Py_Finalize(); 
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

void PyMeshPlugin::slotRunScript()
{
    const QString filename = toolbox_->filename->text();
    OpenFlipperSettings().setValue("Plugin-PyMesh/LastOpenedFile", filename);
    runPyScriptFileAsync(filename, toolbox_->cbClearPrevious->isChecked());
}

////////////////////////////////////////////////////////////////
// Property Conversion

template<template<class> typename Handle, typename T, typename MeshT>
bool createAndCopyProperty(MeshT* mesh, OpenMesh::PropertyT<boost::python::object>* pyProp)
{
    if (!boost::python::extract<T>(*pyProp->data()).check())
        return false;
    // get old handle
    Handle<boost::python::object> pyHandle;
    mesh->get_property_handle(pyHandle, pyProp->name());

    // create new type
    Handle<T> omHandle;
    mesh->add_property(omHandle, pyProp->name());

    // copy
    auto& omProp = mesh->property(omHandle).data_vector();
    auto& propvec = pyProp->data_vector();
    omProp.resize(propvec.size());
    std::transform(propvec.begin(), propvec.end(), omProp.begin(),
        [](boost::python::object& p) -> T {return boost::python::extract<T>(p); });

    // remove old
    mesh->remove_property(pyHandle);
    return true;
}

template<template<class> typename Handle, typename MeshT>
void convertProps(MeshT* mesh, OpenMesh::PropertyT<boost::python::object>* pyProp)
{
    if (createAndCopyProperty<Handle, bool>(mesh, pyProp))
        return;
    if (createAndCopyProperty<Handle, short>(mesh,pyProp))
        return;
    if (createAndCopyProperty<Handle, int>(mesh,pyProp))
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
    std::vector<OpenMesh::PropertyT<boost::python::object>*> props;
    //VProps
    for (auto it = mesh->vprops_begin(); it != mesh->vprops_end(); ++it)
    {
        OpenMesh::PropertyT<boost::python::object>* p = dynamic_cast<OpenMesh::PropertyT<boost::python::object>*>(*it);
        if (p)
            props.push_back(p);
    }

    for (auto p : props)
        convertProps<OpenMesh::VPropHandleT>(mesh, p);
    props.clear();

    //EProps
    for (auto it = mesh->eprops_begin(); it != mesh->eprops_end(); ++it)
    {
        OpenMesh::PropertyT<boost::python::object>* p = dynamic_cast<OpenMesh::PropertyT<boost::python::object>*>(*it);
        if (p)
            props.push_back(p);
    }

    for (auto p : props)
        convertProps<OpenMesh::EPropHandleT>(mesh, p);
    props.clear();

    //FProps
    for (auto it = mesh->fprops_begin(); it != mesh->fprops_end(); ++it)
    {
        OpenMesh::PropertyT<boost::python::object>* p = dynamic_cast<OpenMesh::PropertyT<boost::python::object>*>(*it);
        if (p)
            props.push_back(p);
    }

    for (auto p : props)
        convertProps<OpenMesh::FPropHandleT>(mesh, p);
    props.clear();

    //MProps
    // conversion problems in generic version, write own convert code for mprops
    //for (auto it = mesh->mprops_begin(); it != mesh->mprops_end(); ++it)
    //{
    //    OpenMesh::PropertyT<boost::python::object>* p = dynamic_cast<OpenMesh::PropertyT<boost::python::object>*>(*it);
    //    if (p)
    //        props.push_back(p);
    //}

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

    // clear env
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
        if (PyErr_ExceptionMatches(PyExc_SystemExit))
            PyErr_Clear();  //ignore system exit evaluation, since it will close the whole application
        else
            PyErr_Print();
    else
        Py_XDECREF(result);
    Py_XDECREF(localDictionary);

    PyGILState_Release(state);

    convertPropsPyToCpp_internal(createdObjects_);
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
    for (auto& i : _list)
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
    for (auto& i : _list)
        Q_EMIT updatedObject(i, UPDATE_ALL);
}

void PyMeshPlugin::initPython()
{
    if (Py_IsInitialized())
        return;

    PyImport_AppendInittab("openmesh", openmesh_pyinit_function);

    Py_Initialize();
    PyEval_InitThreads();
    

    main_module_ = boost::python::object(boost::python::import("__main__"));

    // redirect python output
    tyti::pylog::redirect_stderr([this](const char*w) {this->pyError(w); });
    tyti::pylog::redirect_stdout([this](const char* w) {this->pyOutput(w); });

    // add openmesh module    
    boost::python::object main_namespace = main_module_.attr("__dict__");

    boost::python::object om_module(boost::python::import("openmesh"));
    main_namespace["openmesh"] = om_module;

    // hook into mesh constructors
    registerFactoryMethods(this, om_module);

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

OpenMesh::Python::TriMesh* PyMeshPlugin::createTriMesh()
{
    int objectId = -1;

    Q_EMIT addEmptyObject(DATA_TRIANGLE_MESH, objectId);

    TriMeshObject* object;
    PluginFunctions::getObject(objectId, object);

    createdObjects_.push_back(objectId);

    return reinterpret_cast<OpenMesh::Python::TriMesh*>(object->mesh());
}

OpenMesh::Python::PolyMesh* PyMeshPlugin::createPolyMesh()
{
    int objectId = -1;

    Q_EMIT addEmptyObject(DATA_POLY_MESH, objectId);

    PolyMeshObject* object;
    PluginFunctions::getObject(objectId, object);

    createdObjects_.push_back(objectId);

    return reinterpret_cast<OpenMesh::Python::PolyMesh*>(object->mesh());
}


