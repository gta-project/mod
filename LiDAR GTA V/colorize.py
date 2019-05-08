from PIL import Image
from tqdm import tqdm
import sys

im = []
im.append(Image.open('GTA_Camera_Print_0.bmp', 'r'))
im.append(Image.open('GTA_Camera_Print_1.bmp', 'r'))
im.append(Image.open('GTA_Camera_Print_2.bmp', 'r'))
im.append(Image.open('GTA_Camera_Print_3.bmp', 'r'))
im.append(Image.open('GTA_Camera_Print_4.bmp', 'r'))

pix_rgb = []
pix_rgb.append(im[1].load())
pix_rgb.append(im[2].load())
pix_rgb.append(im[3].load())
pix_rgb.append(im[4].load())
pix_rgb.append(im[0].load())

file = open(sys.argv[1], 'r')
n_lines = 0
for x in file:
	n_lines+=1
file.close()

file = open(sys.argv[1], 'r')
color_mapping = {}

output = open(sys.argv[1][:-4] + "_PC.ply","w")
output.write("ply\nformat ascii 1.0\nelement vertex "+str(n_lines)+"\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue\nend_header")


c = 0
for line in tqdm(file, total=n_lines):
    line = line.split()

    coords3d = (line[0],line[1],line[2])
    coords2d = (line[3],line[4])
    #print(coords2d)
    color_mapping[coords2d] = [x for x in coords3d]

    try:
        if line[5] == '0':
    	    r,g,b = pix_rgb[0][int(line[3]),int(line[4])]
    	    #r = 0
        if line[5] == '1':
    	    r,g,b = pix_rgb[1][int(line[3]),int(line[4])]
    	    #r,g = 0,0
        if line[5] == '2':
    	    r,g,b = pix_rgb[2][int(line[3]),int(line[4])]
    	    #r,b = 0,0
        if line[5] == '3':
    	    r,g,b = pix_rgb[3][int(line[3]),int(line[4])]
    	    #g,b = 0,0
        if line[5] == '4':
    	    r,g,b = pix_rgb[4][int(line[3]),int(line[4])]
    	    #b = 0

    except:
        pass

    output.write("\n"+line[0]+" "+line[1]+" "+line[2]+" "+str(r)+" "+str(g)+" "+str(b))
