#include "PyMeshPlugin.hh"


// todo: some switch for boost static build or not
#include <boost/python.hpp>

#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/common/GlobalOptions.hh>

// adds openmesh python module for inter language communication
//#include <OpenMesh/src/Python/Bindings.cc>
#include "OMPyModule.hh"

#include "MeshFactory.hh"

#include <QFileDialog>

static const char* g_job_id = "PyMesh Interpreter";

PyMeshPlugin::PyMeshPlugin():
    main_module_(),
    toolbox_(),
    createdObjects_()
{

}

PyMeshPlugin::~PyMeshPlugin()
{
    Py_Finalize();
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
    for (size_t i = 0; i < propvec.size(); ++i)
        omProp[i] = boost::python::extract<T>(propvec[i]);

    // remove old
    mesh->remove_property(pyHandle);
    mesh->garbage_collection();
    mesh->property_stats();
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
    connect(th, SIGNAL(finished()), this, SLOT(runPyScriptFinished()));

    connect(th, &OpenFlipperThread::function, this, [_script, _clearPrevious, this]()
    {
        this->runPyScript_internal(_script, _clearPrevious);
        Q_EMIT this->finishJob(QString(g_job_id));
    }
    , Qt::DirectConnection);
    // Run Script

    emit startJob(QString(g_job_id), "Runs Python Script", 0, 0, true);
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

    QString jobName = name() + " PyRun Worker";
    OpenFlipperThread* th = new OpenFlipperThread(jobName);
    connect(th, SIGNAL(finished()), this, SLOT(runPyScriptFinished()));

    PyRun_SimpleString(_script.toLatin1());
}

void PyMeshPlugin::runPyScriptFinished()
{
    // Update
    convertPropsPyToCpp(createdObjects_);
    for (size_t i = 0; i < createdObjects_.size(); ++i)
    {
        Q_EMIT updatedObject(createdObjects_[i], UPDATE_ALL);
        Q_EMIT createBackup(createdObjects_[i], "Original Object");
    }

    PluginFunctions::viewAll();
}

void PyMeshPlugin::convertPropsPyToCpp(const IdList& _list)
{
    for (auto& i : _list)
    {
        TriMeshObject* triobj;
        if (PluginFunctions::getObject(i, triobj))
            convertProps(triobj->mesh());
        PolyMeshObject* polyobj;
        if (PluginFunctions::getObject(i, polyobj))
            convertProps(polyobj->mesh());
        Q_EMIT updatedObject(i, UPDATE_ALL);
    }
}

void PyMeshPlugin::initPython()
{
    if (Py_IsInitialized())
        return;

    PyImport_AppendInittab("openmesh", openmesh_pyinit_function);

    Py_Initialize();
    PyEval_InitThreads();

    main_module_ = boost::python::object(boost::python::handle<>(PyImport_AddModule("__main__")));

    // redirect python output
    initPyLogger(main_module_.ptr(), this);

    // add openmesh module    
    boost::python::object main_namespace = main_module_.attr("__dict__");

    boost::python::object om_module((boost::python::handle<>(PyImport_ImportModule("openmesh"))));
    main_namespace["openmesh"] = om_module;

    // hook into mesh constructors
    registerFactoryMethods(this, om_module);
    
}

// Abort functions
int quitPython(void*)
{
    PyErr_SetInterrupt();
    return -1;
}

void PyMeshPlugin::canceledJob(QString _job)
{
    if (!_job.contains(g_job_id))
        return;

    PyGILState_STATE state = PyGILState_Ensure();
    Py_AddPendingCall(&quitPython, NULL);
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


