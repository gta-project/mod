python colorize.py LiDAR_PointCloud_points.txt
IF NOT %ERRORLEVEL%==0 GOTO
python colorize.py LiDAR_PointCloud_error.txt
IF NOT %ERRORLEVEL%==0 GOTO
python colorize.py LiDAR_PointCloud_errorDist.txt
IF NOT %ERRORLEVEL%==0 GOTO
python colorize.py LiDAR_PointCloud_renato.txt
IF NOT %ERRORLEVEL%==0 GOTO
EXIT