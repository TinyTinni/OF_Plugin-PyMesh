# from OpenMesh Tutorial
# http://openmesh.org/Daily-Builds/Doc/a03957.html

from openmesh import *
import numpy as np
import openflipper as ofp

mesh = TriMesh()

vh0 = mesh.add_vertex([0, 1, 0])
vh1 = mesh.add_vertex([1, 0, 0])
vh2 = mesh.add_vertex([2, 1, 0])
vh3 = mesh.add_vertex([0,-1, 0])
vh4 = mesh.add_vertex([2,-1, 0])
print("Points added")

fh0 = mesh.add_face(vh0, vh1, vh2)
fh1 = mesh.add_face(vh1, vh3, vh4)
fh2 = mesh.add_face(vh0, vh3, vh1)
print("faces added")

vh_list = [vh2, vh1, vh4]
fh3 = mesh.add_face(vh_list)

#prop_handle = VPropHandle()
mesh.vertex_property("intProp")

for vh in mesh.vertices():
    mesh.set_vertex_property("intProp", vh, vh.idx())
    
mesh.vertex_property("doubleProp")

for vh in mesh.vertices():
    mesh.set_vertex_property("doubleProp", vh, vh.idx()/(vh.idx()+1))
    
mesh.vertex_property("vectorProp")

for vh in mesh.vertices():
    mesh.set_vertex_property("vectorProp", vh, np.array([vh.idx(),vh.idx()+1,vh.idx()+2]))

# !!! Don't Forget: normals are not updated automatically !!!
mesh.update_normals()

# openflipper module is loaded as ofp
mesh_id = ofp.get_id(mesh)
print("Mesh ID: {}".format(mesh_id))

# build-in openflipper module
for name, mesh in ofp.meshes().items():
    print (name)
	
# experimental function calls over RPC
cube_id = ofp.primitivesgenerator.addCube(np.array([1.0,1.0,1.0]))
# can only be called in gui mode:
#ofp.backup.createBackup(cube_id, "testing_backup", ofp.Update.GEOMETRY | ofp.Update.TOPOLOGY)
cube_mesh = ofp.get_mesh(cube_id)

ofp.core.deleteObject(cube_id)


print("mesh created.")
print("This Message and any errors (if exists) are shown in OF log.")

