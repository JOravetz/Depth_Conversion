import numpy as np
from scipy.interpolate import Rbf

def main():

    nx, ny = 901, 1101

    my_data = np.genfromtxt('input.dat', delimiter=' ',skip_header=0)
    x = my_data[:,0]
    y = my_data[:,1]
    z = my_data[:,2]

    xi = np.linspace(x.min(), x.max(), nx)
    yi = np.linspace(y.min(), y.max(), ny)

    xi, yi = np.meshgrid(xi, yi)
    xi, yi = xi.flatten(), yi.flatten()

    # Calculate scipy's RBF
    grid2 = scipy_idw(x,y,z,xi,yi)
    grid2 = grid2.reshape((ny, nx))

    for i in range ( 0, nx ):
       for j in range ( 0, ny ) :
          print grid2[j,i]

def scipy_idw(x, y, z, xi, yi):
    interp = Rbf(x, y, z, function='linear')
    return interp(xi, yi)

if __name__ == '__main__':
    main()
