/*
* This file is part of PyMesh.
*
* PyMesh is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* PyMesh is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#ifndef PYMESHPLUGIN_HH
#define PYMESHPLUGIN_HH

// !!!!include python related stuff BEFORE QT STUFF!!!!
// cannot use QT_NO_KEYWORDS, OF Interface Headers dont support it
// disable python debug build
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#include "PyLogHook/PyLogHook.h"

// !!! Qt Stuff starts here !!!
#include <QObject>

#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>
#include <OpenFlipper/BasePlugin/ToolboxInterface.hh>
#include <OpenFlipper/BasePlugin/LoadSaveInterface.hh>
#include <OpenFlipper/BasePlugin/BackupInterface.hh>
#include <OpenFlipper/BasePlugin/ProcessInterface.hh>

#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>

#include <OpenMesh/src/Python/Bindings.hh>

#include <vector>

#include "PyMeshToolbox.hh"


class PyMeshPlugin : public QObject, BaseInterface, ToolboxInterface, LoadSaveInterface, LoggingInterface, BackupInterface, ProcessInterface
{
  Q_OBJECT
  Q_INTERFACES(BaseInterface)
  Q_INTERFACES(ToolboxInterface)
  Q_INTERFACES(LoadSaveInterface)
  Q_INTERFACES(LoggingInterface)
  Q_INTERFACES(BackupInterface)
  Q_INTERFACES(ProcessInterface)
  Q_PLUGIN_METADATA(IID "org.OpenFlipper.Plugins.Plugin-PyMesh")

Q_SIGNALS:

  // Base Interface
  void updatedObject(int _identifier, const UpdateType& _type);

  // Toolbox Interface
  void addToolbox(QString  _name, QWidget* _widget , QIcon* _icon);

  // Load-Save Interface
  void addEmptyObject(DataType _type, int& _id);
  void deleteObject(int _id);

  // Logging Interface
  void log(Logtype _type, QString _message);
  void log(QString _message);

  // Process Interface
  void setJobDescription(QString _jobId, QString _text);
  void startJob(QString _jobId, QString _description, int _min, int _max, bool _blocking = false);
  void finishJob(QString _jobId);

  // Backup Interface
  void createBackup(int _objectid, QString _name, UpdateType _type = UPDATE_ALL);

public Q_SLOTS:

  void runPyScriptFile(const QString& _filename, bool _clearPrevious);
  void runPyScriptFileAsync(const QString& _filename, bool _clearPrevious);

  void runPyScript(const QString& _script, bool _clearPrevious);
  void runPyScriptAsync(const QString& _script, bool _clearPrevious);

  void convertPropsPyToCpp(const IdList& _list);

public:

  // Python callback functions
  void pyOutput(const char* w);
  void pyError(const char* w);  
  OpenMesh::Python::TriMesh* createTriMesh();
  OpenMesh::Python::PolyMesh* createPolyMesh();


  PyMeshPlugin();
  ~PyMeshPlugin();

  QString name(){return QString("PyMeshPlugin");}
  QString description(){return QString("Run OpenMesh Python Scripts and shows the resulting mesh.");}

private:

    boost::python::object main_module_;

    PyMeshToolbox* toolbox_;
    std::vector<int> createdObjects_;

    void initPython();
    /// Run Python Script. Does not update any object. save to call from another thread
    void runPyScript_internal(const QString& _script, bool _clearPrevious);
    /// Does not update all objects, save to call from another thread
    void convertPropsPyToCpp_internal(const IdList& _list);

private Q_SLOTS:

  void initializePlugin();

  void pluginsInitialized();

  void slotRunScript();

  void slotSelectFile();

  void canceledJob(QString _job);

  void runPyScriptFinished();

public Q_SLOTS:
  QString version(){ return QString("1.0"); }
};


#endif //PYMESHPLUGIN_HH
