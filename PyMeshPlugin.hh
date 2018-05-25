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

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/pytypes.h>

#include <QObject>

#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>
#include <OpenFlipper/BasePlugin/ToolboxInterface.hh>
#include <OpenFlipper/BasePlugin/LoadSaveInterface.hh>
#include <OpenFlipper/BasePlugin/BackupInterface.hh>
#include <OpenFlipper/BasePlugin/ProcessInterface.hh>
#include <OpenFlipper/BasePlugin/ScriptInterface.hh>
#include <OpenFlipper/BasePlugin/RPCInterface.hh>

#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>

#include <vector>


#include "PyMeshToolbox.hh"


class PyMeshPlugin : public QObject, BaseInterface, ToolboxInterface, LoadSaveInterface, LoggingInterface, BackupInterface, ProcessInterface, ScriptInterface, RPCInterface
{
  Q_OBJECT
  Q_INTERFACES(BaseInterface)
  Q_INTERFACES(ToolboxInterface)
  Q_INTERFACES(LoadSaveInterface)
  Q_INTERFACES(LoggingInterface)
  Q_INTERFACES(BackupInterface)
  Q_INTERFACES(ProcessInterface)
  Q_INTERFACES(ScriptInterface)
  Q_INTERFACES(RPCInterface)
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

  // Script Interface
  void getAvailableFunctions(QStringList& _functions);
  void getDescription(QString _function, QString& _description, QStringList& _parameters, QStringList& _descriptions);

  // Backup Interface
  void createBackup(int _objectid, QString _name, UpdateType _type = UPDATE_ALL);

public Q_SLOTS:

  bool runPyScriptFile(const QString& _filename, bool _clearPrevious);
  void runPyScriptFileAsync(const QString& _filename, bool _clearPrevious);

  bool runPyScript(const QString& _script, bool _clearPrevious);
  void runPyScriptAsync(const QString& _script, bool _clearPrevious);

  void convertPropsPyToCpp(const IdList& _list);

  void resetInterpreter();

public:

  // Python callback functions
  void pyOutput(const char* w);
  void pyError(const char* w);  
  void* createTriMesh();
  void* createPolyMesh();


  PyMeshPlugin();
  ~PyMeshPlugin();

  QString name(){return QString("PyMesh");}
  QString description(){return QString("Run OpenMesh Python Scripts and shows the resulting mesh.");}

private:

    pybind11::module main_module_;
    PyObject* global_dict_clean_ = nullptr; //clean global dict. used for reset. do not change

    PyMeshToolbox* toolbox_ = nullptr;
    std::vector<int> createdObjects_;

    QWidget* scriptingFunctionsPresenter_ = nullptr;

    void initPython();
    /// Run Python Script. Does not update any object. save to call from another thread
    bool runPyScript_internal(const QString& _script, bool _clearPrevious);
    /// Does not update all objects, save to call from another thread
    void convertPropsPyToCpp_internal(const IdList& _list);

private Q_SLOTS:

  void initializePlugin();

  void pluginsInitialized();

  void slotRunScript();

  void slotSelectFile();

  void canceledJob(QString _job);

  void runPyScriptFinished();

  void noguiSupported() {};

  void showScriptingFunctions();

  void slotconvertPropsPyToCpp();

public Q_SLOTS:
  QString version(){ return QString("2.0"); }
};


#endif //PYMESHPLUGIN_HH
