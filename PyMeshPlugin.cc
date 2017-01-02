#include "PyMeshPlugin.hh"


// todo: some switch for boost static build or not
#include <boost/python.hpp>

#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/common/GlobalOptions.hh>

// adds openmesh python module for inter language communication
#include <OpenMesh/src/Python/Bindings.cc>

#include "MeshFactory.hh"

#include <QFileDialog>

//fix for msvc 2015 update 3, delete in future
namespace boost
{
    template <>
    OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> const volatile * get_pointer<OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> const volatile >(
        OpenMesh::TriMesh_ArrayKernelT<struct OpenMesh::Python::MeshTraits> const volatile *c)
    {
        return c;
    }

}


PyMeshPlugin::PyMeshPlugin()
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
    Q_EMIT addToolbox("PyMesh", toolbox_);

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
    runPyScriptFile(filename, toolbox_->cbClearPrevious->isChecked());
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


void PyMeshPlugin::runPyScript(const QString& _script, bool _clearPrevious)
{
    // init
    initPython();

    // clear env
    if (_clearPrevious)
        for (size_t i = 0; i < m_createdObjects.size(); ++i)
            Q_EMIT deleteObject(m_createdObjects[i]);
    m_createdObjects.clear();

    // Run Script
    PyRun_SimpleString(_script.toLatin1());

    // Update
    for (size_t i = 0; i < m_createdObjects.size(); ++i)
    {
        Q_EMIT updatedObject(m_createdObjects[i], UPDATE_ALL);
        Q_EMIT createBackup(m_createdObjects[i], "Original Object");
    }

    PluginFunctions::viewAll();
}

void PyMeshPlugin::initPython()
{
    if (Py_IsInitialized())
        return;

#if (PY_MAJOR_VERSION == 2)
    PyImport_AppendInittab("openmesh", &OpenMesh::Python::initopenmesh); //untested
#elif (PY_MAJOR_VERSION == 3)
    PyImport_AppendInittab("openmesh", &OpenMesh::Python::PyInit_openmesh);
#endif

    Py_Initialize();

    boost::python::object main_module(boost::python::handle<>(PyImport_AddModule("__main__")));

    // redirect python output
    initPyLogger(main_module.ptr(), this);

    // add openmesh module    
    boost::python::object main_namespace = main_module.attr("__dict__");

    boost::python::object om_module((boost::python::handle<>(PyImport_ImportModule("openmesh"))));
    main_namespace["openmesh"] = om_module;

    // hook into mesh constructors
    registerFactoryMethods(this, om_module);
    
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

    m_createdObjects.push_back(objectId);

    return reinterpret_cast<OpenMesh::Python::TriMesh*>(object->mesh());
}

OpenMesh::Python::PolyMesh* PyMeshPlugin::createPolyMesh()
{
    int objectId = -1;

    Q_EMIT addEmptyObject(DATA_POLY_MESH, objectId);

    PolyMeshObject* object;
    PluginFunctions::getObject(objectId, object);

    m_createdObjects.push_back(objectId);

    return reinterpret_cast<OpenMesh::Python::PolyMesh*>(object->mesh());
}


