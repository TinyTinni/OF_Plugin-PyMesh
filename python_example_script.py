# from OpenMesh Tutorial
# http://openmesh.org/Daily-Builds/Doc/a03957.html
from openmesh import *

mesh = TriMesh()

vh0 = mesh.add_vertex(TriMesh.Point(0, 1, 0))
vh1 = mesh.add_vertex(TriMesh.Point(1, 0, 0))
vh2 = mesh.add_vertex(TriMesh.Point(2, 1, 0))
vh3 = mesh.add_vertex(TriMesh.Point(0,-1, 0))
vh4 = mesh.add_vertex(TriMesh.Point(2,-1, 0))

fh0 = mesh.add_face(vh0, vh1, vh2)
fh1 = mesh.add_face(vh1, vh3, vh4)
fh2 = mesh.add_face(vh0, vh3, vh1)

vh_list = [vh2, vh1, vh4]
fh3 = mesh.add_face(vh_list)

prop_handle = VPropHandle()
mesh.add_property(prop_handle, "cogs")

for vh in mesh.vertices():
    cog = TriMesh.Point(0,0,0)
    valence = 0
    for neighbor in mesh.vv(vh):
        cog += mesh.point(neighbor)
        valence += 1
    mesh.set_property(prop_handle, vh, cog / valence)

# !!! Don't Forget: normals are not updated automatically !!!
mesh.update_normals()

# build-in openflipper module
for name, mesh in openflipper.meshes().items():
    print (name)

print("mesh created.")
print("This Message and any errors (if exists) are shown in OF log.")

