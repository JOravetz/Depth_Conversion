import sys
import numpy as np
from scipy.interpolate import Rbf

def main():
    # Setup: Generate data...

    nx, ny, nt = 901, 1101, 5

    filename = sys.argv[-1]
    my_data = np.genfromtxt ( filename, delimiter=' ',skip_header=0 )
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

    # Calculate scipy's RBF
    grid2 = scipy_idw(x,y,t,z,xi,yi,ti)
    grid2 = grid2.reshape((ny, nx, nt))

    for j in range ( 0, nx ):
       for k in range ( 0, ny ) :
          print ti_orig[2], xi_orig[j], yi_orig[k], grid2[k,j,2]

def scipy_idw(x, y, t, z, xi, yi, ti):
    interp = Rbf(x, y, t, z, function='linear')
    return interp(xi, yi, ti)

if __name__ == '__main__':
    main()
