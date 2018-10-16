import cv2
import numpy as np
import os

class KDtree:
    def __init__(self, dim=-1, dvalue=np.uint8(0), ltree=None, rtree=None):
        self.dim = dim 
        self.dvalue = dvalue 
        self.ltree = ltree 
        self.rtree = rtree 
        self.point = None
        self.code = 0
        self.divide_bit = 0

    def LUT(self, code):
        if self.dim == -1: return self.point
        
        if (code & self.divide_bit) == 0: # left
            if type(self.ltree) == type(self):
                return self.ltree.LUT(code)
            else:
                return self.point
        else:
            if type(self.rtree) == type(self):
                return self.rtree.LUT(code)
            else:
                return self.point

    def encode_point(self, point):
        assert point.size > self.dim
        
        if self.dim == -1:
            return self.point, self.code

        if point[self.dim] <= self.dvalue:
            if type(self.ltree) == type(self):
                return self.ltree.encode_point(point)
            else:
                return self.point, self.code
        else:
            if type(self.rtree) == type(self):
                return self.rtree.encode_point(point)
            else:
                return self.point, (self.code | self.divide_bit) 
    
    def print_tree(self, level=0):
        print(''.join([' ' for x in range(level)]), end='')
        print(self.dim, self.point)
        if self.dim == -1:
            return

        if type(self.ltree) == type(self):
            self.ltree.print_tree(level+1)
        else:
            print(''.join([' ' for x in range(level+1)]), end='')
            print("<null>")

        if type(self.rtree) == type(self):
            self.rtree.print_tree(level+1)
        else:
           print(''.join([' ' for x in range(level+1)]), end='')
           print("<null>")


def build_kdtree(kdtree, points, deeplimit, dimorder=None):
    """
    This function will build k-dim tree with the point set.
    """
    assert len(points.shape) == 2
    assert deeplimit >= 0
    if type(dimorder) != type(None):
        assert len(dimorder) >= deeplimit

    if deeplimit != 0: kdtree.divide_bit = 1 << (deeplimit-1)
    # deal with deeplimit == 0, leaf node.
    if points.shape[0] <= 1 or deeplimit == 0:
        kdtree.dim = -1
        kdtree.point = np.uint8(np.mean(points, axis=0))
        return


    # decide divide dim
    dividedim = -1
    if type(dimorder) == type(None):
        # compute cubebbox for points.
        cubebbox = np.array([[np.min(x), np.max(x)] for x in points.T], dtype=np.uint8)

        # find long dim in cube.
        dividedim = np.argmax(cubebbox[:,1]-cubebbox[:,0])
    else:
        # use dim in dimorder
        dividedim = dimorder[len(dimorder)-deeplimit]

    assert dividedim != -1

    sorted_dim_idx = np.argsort(points[:,dividedim])

    # compute median of the dim dvalue.
    median = 0
    if sorted_dim_idx.size % 2 == 0:
        median_idx1 = sorted_dim_idx[sorted_dim_idx.size//2]
        median_idx2 = sorted_dim_idx[sorted_dim_idx.size//2-1]
        median = np.uint8((int(points[median_idx1, dividedim]) + int(points[median_idx2, dividedim]))/2)
    else:
        median_idx = sorted_dim_idx[sorted_dim_idx.size//2]
        median = points[median_idx, dividedim]

    # find divide index in the dim of cube.
    divide_idx = sorted_dim_idx.size
    for i in range(sorted_dim_idx.size):
        if points[sorted_dim_idx[i], dividedim] > median:
            divide_idx = i
            break

    # compute points set 1 and 2 with point set's cube bbox.
    points1 = points[sorted_dim_idx[:divide_idx], :]
    points2 = points[sorted_dim_idx[divide_idx:], :]

    # build tree
    kdtree.dim = dividedim
    kdtree.dvalue = median
    kdtree.point = np.uint8(np.mean(points, axis=0))

    if points1.size != 0:
        kdtree.ltree = KDtree()
        build_kdtree(kdtree.ltree, points1, deeplimit-1, dimorder)
        kdtree.lcode = kdtree.ltree.code

    if points2.size != 0:
        if deeplimit > 6:
            t = 0
        kdtree.rtree = KDtree()
        kdtree.rtree.code = kdtree.code | kdtree.divide_bit
        build_kdtree(kdtree.rtree, points2, deeplimit-1, dimorder)
        kdtree.rcode = kdtree.rtree.code


