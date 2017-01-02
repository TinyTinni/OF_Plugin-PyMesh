# from OpenMesh Tutorial
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

# !!! Don't Forget: normals are not updated automatically !!!
mesh.update_normals()

