import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import Rbf
from mpl_toolkits.mplot3d import Axes3D

def main():
    # Setup: Generate data...

    nx, ny, nt = 901, 1101, 5

    my_data = np.genfromtxt('test1.dat', delimiter=' ',skip_header=0)
    x = my_data[:,0]
    y = my_data[:,1]
    t = my_data[:,2]
    z = my_data[:,3]

    xi = np.linspace(x.min(), x.max(), nx)
    yi = np.linspace(y.min(), y.max(), ny)
    ti = np.linspace(t.min(), t.max(), nt)

    xi_orig = np.linspace(x.min(), x.max(), nx)
    yi_orig = np.linspace(y.min(), y.max(), ny)
    ti_orig = np.linspace(t.min(), t.max(), nt)

    xi, yi, ti = np.meshgrid(xi, yi, ti)
    xi, yi, ti = xi.flatten(), yi.flatten(), ti.flatten()

    # Calculate IDW
    # grid1 = simple_idw(x,y,t,z,xi,yi,ti)
    # grid1 = grid1.reshape((ny, nx, nt))

    # Calculate scipy's RBF
    grid2 = scipy_idw(x,y,t,z,xi,yi,ti)
    grid2 = grid2.reshape((ny, nx, nt))

    for j in range ( 0, nx ):
       for k in range ( 0, ny ) :
          print ti_orig[2], xi_orig[j], yi_orig[k], grid2[k,j,2]

    # grid3 = linear_rbf(x,y,t,z,xi,yi,ti)
    # grid3 = grid3.reshape((ny, nx, nt))

    # Comparisons...
    ### plot(x,y,z,grid1)
    ### plt.title('Homemade IDW')

    ### plot(x,y,t,z,grid2)
    ### plt.title("Scipy's Rbf with function=linear")

    ### plot(x,y,z,grid3)
    ### plt.title('Homemade linear Rbf')

    # fig = plt.figure()
    # ax = fig.add_subplot(111, projection='3d')

    # ax.scatter(xi, yi, ti, c=grid2, cmap=plt.hot())

    # plt.show()

def simple_idw(x, y, t, z, xi, yi, ti):
    dist = distance_matrix(x,y,t, xi,yi,ti)

    # In IDW, weights are 1 / distance
    weights = 1.0 / dist

    # Make weights sum to one
    weights /= weights.sum(axis=0)

    # Multiply the weights for each interpolated point by all observed Z-values
    zi = np.dot(weights.T, z)
    return zi

def linear_rbf(x, y, t, z, xi, yi, ti):
    dist = distance_matrix(x,y,t, xi,yi,ti)

    # Mutual pariwise distances between observations
    internal_dist = distance_matrix(x,y,t, x,y,t)

    # Now solve for the weights such that mistfit at the observations is minimized
    weights = np.linalg.solve(internal_dist, z)

    # Multiply the weights for each interpolated point by the distances
    zi =  np.dot(dist.T, weights)
    return zi

def scipy_idw(x, y, t, z, xi, yi, ti):
    interp = Rbf(x, y, t, z, function='linear')
    return interp(xi, yi, ti)

def distance_matrix(x0, y0, t0, x1, y1, t1):
    obs = np.vstack((x0, y0, t0)).T
    interp = np.vstack((x1, y1, t1)).T

    # Make a distance matrix between pairwise observations
    # Note: from < http://stackoverflow.com/questions/1871536> 
    # (Yay for ufuncs!)
    d0 = np.subtract.outer(obs[:,0], interp[:,0])
    d1 = np.subtract.outer(obs[:,1], interp[:,1])

    return np.hypot(d0, d1)

def plot(x,y,z,grid):
    plt.figure()
    plt.imshow(grid, extent=(x.min(), x.max(), y.max(), y.min()))
    plt.hold(True)
    plt.scatter(x,y,c=z)
    plt.colorbar()

if __name__ == '__main__':
    main()
