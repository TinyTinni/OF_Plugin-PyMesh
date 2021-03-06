import unittest
import openmesh

import numpy as np

class Normals(unittest.TestCase):
    
    def setUp(self):
        self.mesh = openmesh.TriMesh()
        
        # Add some vertices
        self.vhandle = []
        
        self.vhandle.append(self.mesh.add_vertex(np.array([0, 0, 0])))
        self.vhandle.append(self.mesh.add_vertex(np.array([0, 1, 0])))
        self.vhandle.append(self.mesh.add_vertex(np.array([1, 1, 0])))
        self.vhandle.append(self.mesh.add_vertex(np.array([0, 0, 1])))
        
        # Add four faces
        face_vhandles = []
        
        face_vhandles.append(self.vhandle[0])
        face_vhandles.append(self.vhandle[1])
        face_vhandles.append(self.vhandle[2])
        self.mesh.add_face(face_vhandles)
        
        face_vhandles = []
        
        face_vhandles.append(self.vhandle[0])
        face_vhandles.append(self.vhandle[2])
        face_vhandles.append(self.vhandle[3])
        self.mesh.add_face(face_vhandles)
        
        face_vhandles = []
        
        face_vhandles.append(self.vhandle[2])
        face_vhandles.append(self.vhandle[1])
        face_vhandles.append(self.vhandle[3])
        self.mesh.add_face(face_vhandles)
        
        face_vhandles = []
        
        face_vhandles.append(self.vhandle[3])
        face_vhandles.append(self.vhandle[1])
        face_vhandles.append(self.vhandle[0])
        self.mesh.add_face(face_vhandles)

    def test_normal_calculations(self):
        # Check one Request only vertex normals
        # Face normals are required for vertex and halfedge normals, so
        # that prevent access to non existing properties are in place
    
        self.mesh.request_vertex_normals()
        self.mesh.request_halfedge_normals()

        # Check blocks
        self.mesh.update_normals()

        # Request required face normals
        self.mesh.request_face_normals()

        # Automatically compute all normals
        # As only vertex normals are requested and no face normals, this will compute nothing.
        self.mesh.update_normals()

        # Face normals alone
        self.mesh.update_face_normals()

        # Vertex normals alone (require valid face normals)
        self.mesh.update_vertex_normals()

        # Halfedge normals alone (require valid face normals)
        self.mesh.update_halfedge_normals()

    def test_calc_vertex_normal_fast(self):
        n = self.mesh.calc_vertex_normal_fast(self.vhandle[2])
        self.assertTrue(np.allclose(n, np.array([0.70710678, 0., -0.29289322])))

    def test_calc_vertex_normal_correct(self):
        n = self.mesh.calc_vertex_normal_correct(self.vhandle[2])
        self.assertTrue(np.allclose(n, np.array([1, 0, 0])))

    def test_calc_vertex_normal_loop(self):
        n = self.mesh.calc_vertex_normal_loop(self.vhandle[2])
        self.assertTrue(np.allclose(n, np.array([0.8660254, 0, 0])))


if __name__ == '__main__':
    suite = unittest.TestLoader().loadTestsFromTestCase(Normals)
    unittest.TextTestRunner(verbosity=2).run(suite)
