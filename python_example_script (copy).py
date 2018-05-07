print("Hello")

# from OpenMesh Tutorial
# http://openmesh.org/Daily-Builds/Doc/a03957.html
from openmesh import *

mesh = TriMesh(1)
print("created1")
print(type(mesh))
print(type(TriMesh.__init__))
#mesh2 = createTriMesh()
#print("created2")
#print(type(mesh2))
#print(type(createTriMesh))
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

#vh_list = [vh2, vh1, vh4]
#fh3 = mesh.add_face(vh_list)

#prop_handle = VPropHandle()
#mesh.add_property(prop_handle, "cogs")

#for vh in mesh.vertices():
#    cog = TriMesh.Point(0,0,0)
#    valence = 0
#    for neighbor in mesh.vv(vh):
#        cog += mesh.point(neighbor)
#        valence += 1
#    mesh.set_property(prop_handle, vh, cog / valence)

# !!! Don't Forget: normals are not updated automatically !!!
#mesh.update_normals()

# build-in openflipper module
#for name, mesh in openflipper.meshes().items():
#    print (name)
	
# experimental function calls over RPC
#cube_id = openflipper.rpc_call("primitivesgenerator","addCube")
#cube_mesh = openflipper.get_mesh(cube_id)
#openflipper.rpc_call("core","deleteObject",["int",cube_id])


print("mesh created.")
print("This Message and any errors (if exists) are shown in OF log.")

