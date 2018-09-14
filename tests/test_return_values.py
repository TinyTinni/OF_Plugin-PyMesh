from openmesh import *
import openflipper as ofp
import numpy as np

mesh = TriMesh()

vh0 = mesh.add_vertex([0, 1, 0])
vh1 = mesh.add_vertex([1, 0, 0])
vh2 = mesh.add_vertex([2, 1, 0])
vh3 = mesh.add_vertex([0,-1, 0])
vh4 = mesh.add_vertex([2,-1, 0])

id = ofp.get_id(mesh)

bbmax = ofp.infomeshobject.boundingBoxMax(id)
bbmin = ofp.infomeshobject.boundingBoxMin(id)

if not np.allclose(bbmax, [2.,1.,0.]):
    raise RuntimeError("bbmax is {}".format(bbmax))
if not np.allclose(bbmin, [0.,-1.,0.]):
    raise RuntimeError("bbmin is {}.".format(bbmin))
