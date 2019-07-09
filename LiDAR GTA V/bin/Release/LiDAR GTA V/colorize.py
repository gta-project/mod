from PIL import Image
from tqdm import tqdm
import sys

imDay = []
imDay.append(Image.open('LiDAR_PointCloud_Camera_Print_Day_0.bmp', 'r'))
imDay.append(Image.open('LiDAR_PointCloud_Camera_Print_Day_1.bmp', 'r'))
imDay.append(Image.open('LiDAR_PointCloud_Camera_Print_Day_2.bmp', 'r'))

imNight = []
imNight.append(Image.open('LiDAR_PointCloud_Camera_Print_Night_0.bmp', 'r'))
imNight.append(Image.open('LiDAR_PointCloud_Camera_Print_Night_1.bmp', 'r'))
imNight.append(Image.open('LiDAR_PointCloud_Camera_Print_Night_2.bmp', 'r'))

imCloudy = []
imCloudy.append(Image.open('LiDAR_PointCloud_Camera_Print_Cloudy_0.bmp', 'r'))
imCloudy.append(Image.open('LiDAR_PointCloud_Camera_Print_Cloudy_1.bmp', 'r'))
imCloudy.append(Image.open('LiDAR_PointCloud_Camera_Print_Cloudy_2.bmp', 'r'))

pix_rgb_day = []
pix_rgb_day.append(imDay[0].load())
pix_rgb_day.append(imDay[1].load())
pix_rgb_day.append(imDay[2].load())

pix_rgb_night = []
pix_rgb_night.append(imNight[0].load())
pix_rgb_night.append(imNight[1].load())
pix_rgb_night.append(imNight[2].load())

pix_rgb_cloudy = []
pix_rgb_cloudy.append(imCloudy[0].load())
pix_rgb_cloudy.append(imCloudy[1].load())
pix_rgb_cloudy.append(imCloudy[2].load())

file = open(sys.argv[1], 'r')
n_lines = 0
for x in file:
    n_lines+=1
file.close()

file = open(sys.argv[1], 'r')
color_mapping = {}

with open(sys.argv[1][:-4] + "_PC_Day.ply","w") as day, open(sys.argv[1][:-4] + "_PC_Night.ply","w") as night, open(sys.argv[1][:-4] + "_PC_Cloudy.ply","w") as cloudy:
    day.write("ply\nformat ascii 1.0\nelement vertex "+str(n_lines)+"\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue\nend_header")
    night.write("ply\nformat ascii 1.0\nelement vertex "+str(n_lines)+"\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue\nend_header")
    cloudy.write("ply\nformat ascii 1.0\nelement vertex "+str(n_lines)+"\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue\nend_header")

    print("Colorizing point cloud...")
    for line in tqdm(file, total=n_lines):
        line = line.split()

        coords3d = (line[0],line[1],line[2])
        coords2d = (line[3],line[4])
        color_mapping[coords2d] = [x for x in coords3d]

        try:
            if line[5] == '0':
                rgbDay = pix_rgb_day[0][int(line[3]),int(line[4])]
                rgbNight = pix_rgb_night[0][int(line[3]),int(line[4])]
                rgbCloudy = pix_rgb_cloudy[0][int(line[3]),int(line[4])]
                #r = 0
            if line[5] == '1':
                rgbDay = pix_rgb_day[1][int(line[3]),int(line[4])]
                rgbNight = pix_rgb_night[1][int(line[3]),int(line[4])]
                rgbCloudy = pix_rgb_cloudy[1][int(line[3]),int(line[4])]
                #g = 0
            if line[5] == '2':
                rgbDay = pix_rgb_day[2][int(line[3]),int(line[4])]
                rgbNight = pix_rgb_night[2][int(line[3]),int(line[4])]
                rgbCloudy = pix_rgb_cloudy[2][int(line[3]),int(line[4])]
                #b = 0
                
            day.write("\n"+line[0]+" "+line[1]+" "+line[2]+" "+str(rgbDay[0])+" "+str(rgbDay[1])+" "+str(rgbDay[2]))
            night.write("\n"+line[0]+" "+line[1]+" "+line[2]+" "+str(rgbNight[0])+" "+str(rgbNight[1])+" "+str(rgbNight[2]))
            cloudy.write("\n"+line[0]+" "+line[1]+" "+line[2]+" "+str(rgbCloudy[0])+" "+str(rgbCloudy[1])+" "+str(rgbCloudy[2]))
        except:
            pass