#define _USE_MATH_DEFINES
#include "script.h"
#include "keyboard.h"
#include <string>
#include <ctime>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <windows.h>
#include <gdiplus.h>
#include <iterator>
#include <random>
#include "direct.h"

#pragma comment(lib, "gdiplus.lib")

#pragma warning(disable : 4244 4305) // double <-> float conversions

#define StopSpeed 0.00000000001f
#define NormalSpeed 1.0f

static std::default_random_engine generator;
static std::uniform_real_distribution<double> distribution(-1.0, 1.0);

using namespace Gdiplus;

std::vector<double> split(const std::string& s, char delimiter)
{
	std::vector<double> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(std::stod(token));
	}
	return tokens;
}

void readErrorFile(std::vector<double>& dist, std::vector<double>& error) {
	std::ifstream inputFile;
	inputFile.open("LiDAR GTA V/dist_error.csv");
	std::string errorDists, dists;
	inputFile >> errorDists;
	inputFile >> dists;

	dist = split(dists, ',');
	error = split(errorDists, ',');
}

double getError(std::vector<double>& dist, std::vector<double>& error, double r) {
	int x = 0;
	if (r == 0)
		return 0.0;
	int tmp = abs(r);
	while (dist[x] <= tmp) {
		if (dist[x] == tmp)
			return error[x];
		x++;
	}
	double fact;
	if (x == 0) {
		return 0;
	}
	fact = (abs(r) - dist[x - 1]) / (dist[x] - dist[x - 1]);
	return (1 - fact) * error[x - 1] + (fact)* error[x];
}

void introduceError(Vector3* xyzError, double x, double y, double z, double error, int errorType, int range, std::vector<double>& dist_vector, std::vector<double>& error_vector)
{
	//Convert to spherical coordinates
	double hxy = std::hypot(x, y);
	double r = std::hypot(hxy, z);
	double el = std::atan2(z, hxy);
	double az = std::atan2(y, x);

	if (errorType == 0)
		r = r + distribution(generator) * error;
	else if (errorType == 1)
		r = r + distribution(generator) * error * r;
	else {
		if (r != 0)
			r = r + distribution(generator) * getError(dist_vector, error_vector, r);
	}

	//Convert to cartesian coordinates
	double rcos_theta = r * std::cos(el);
	xyzError->x = rcos_theta * std::cos(az);
	xyzError->y = rcos_theta * std::sin(az);
	xyzError->z = r * std::sin(el);
}

void notificationOnLeft(std::string notificationText) {
	UI::_SET_NOTIFICATION_TEXT_ENTRY("CELL_EMAIL_BCON");
	const int maxLen = 99;
	for (int i = 0; i < notificationText.length(); i += maxLen) {
		std::string divideText = notificationText.substr(i, min(maxLen, notificationText.length() - i));
		const char* divideTextAsConstCharArray = divideText.c_str();
		char* divideTextAsCharArray = new char[divideText.length() + 1];
		strcpy_s(divideTextAsCharArray, divideText.length() + 1, divideTextAsConstCharArray);
		UI::_ADD_TEXT_COMPONENT_STRING(divideTextAsCharArray);
	}
	int handle = UI::_DRAW_NOTIFICATION(false, 1);
}

struct ray {
	bool hit;
	Vector3 hitCoordinates;
	Vector3 surfaceNormal;
	std::string entityTypeName;
	int rayResult;
	int hitEntityHandle;
};

ray raycast(Vector3 source, Vector3 direction, float maxDistance, int intersectFlags) {
	ray result;
	float targetX = source.x + (direction.x * maxDistance);
	float targetY = source.y + (direction.y * maxDistance);
	float targetZ = source.z + (direction.z * maxDistance);
	int rayHandle = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(source.x, source.y, source.z, targetX, targetY, targetZ, intersectFlags, PLAYER::PLAYER_PED_ID(), 7);
	int hit = 0;
	int hitEntityHandle = -1;
	Vector3 hitCoordinates;
	hitCoordinates.x = 0;
	hitCoordinates.y = 0;
	hitCoordinates.z = 0;
	Vector3 surfaceNormal;
	surfaceNormal.x = 0;
	surfaceNormal.y = 0;
	surfaceNormal.z = 0;
	int rayResult = WORLDPROBE::_GET_RAYCAST_RESULT(rayHandle, &hit, &hitCoordinates, &surfaceNormal, &hitEntityHandle);
	result.rayResult = rayResult;
	result.hit = hit;
	result.hitCoordinates = hitCoordinates;
	result.surfaceNormal = surfaceNormal;
	result.hitEntityHandle = hitEntityHandle;
	std::string entityTypeName = "Unknown";
	if (ENTITY::DOES_ENTITY_EXIST(hitEntityHandle)) {
		int entityType = ENTITY::GET_ENTITY_TYPE(hitEntityHandle);
		if (entityType == 1) {
			entityTypeName = "GTA.Ped";
		}
		else if (entityType == 2) {
			entityTypeName = "GTA.Vehicle";
		}
		else if (entityType == 3) {
			entityTypeName = "GTA.Prop";
		}
	}
	result.entityTypeName = entityTypeName;
	return result;
}

ray angleOffsetRaycast(double angleOffsetX, double angleOffsetZ, int range) {
	Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(2);
	double rotationX = (rot.x + angleOffsetX) * (M_PI / 180.0);
	double rotationZ = (rot.z + angleOffsetZ) * (M_PI / 180.0);
	double multiplyXY = abs(cos(rotationX));
	Vector3 direction;
	direction.x = sin(rotationZ) * multiplyXY * -1;
	direction.y = cos(rotationZ) * multiplyXY;
	direction.z = sin(rotationX);
	ray result = raycast(ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0, 0, 1.2), direction, range, -1);
	return result;
}

int GetEncoderClsid(WCHAR* format, CLSID* pClsid)
{
	unsigned int num = 0, size = 0;
	GetImageEncodersSize(&num, &size);
	if (size == 0) return -1;
	ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL) return -1;
	GetImageEncoders(num, size, pImageCodecInfo);

	for (unsigned int j = 0; j < num; ++j) {
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}
	free(pImageCodecInfo);
	return -1;
}

int SaveScreenshot(std::string filename, ULONG uQuality = 100)
{
	ULONG_PTR gdiplusToken;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	HWND hMyWnd = GetDesktopWindow();
	RECT r;
	int w, h;
	HDC dc, hdcCapture;
	int nBPP, nCapture, iRes;
	LPBYTE lpCapture;
	CLSID imageCLSID;
	Bitmap* pScreenShot;

	// get the area of my application's window     
	GetWindowRect(hMyWnd, &r);
	dc = GetWindowDC(hMyWnd);   // GetDC(hMyWnd) ;
	w = r.right - r.left;
	h = r.bottom - r.top;
	nBPP = GetDeviceCaps(dc, BITSPIXEL);
	hdcCapture = CreateCompatibleDC(dc);

	// create the buffer for the screenshot
	BITMAPINFO bmiCapture = { sizeof(BITMAPINFOHEADER), w, -h, 1, nBPP, BI_RGB, 0, 0, 0, 0, 0, };

	// create a container and take the screenshot
	HBITMAP hbmCapture = CreateDIBSection(dc, &bmiCapture, DIB_PAL_COLORS, (LPVOID*)& lpCapture, NULL, 0);

	// failed to take it
	if (!hbmCapture) {
		DeleteDC(hdcCapture);
		DeleteDC(dc);
		GdiplusShutdown(gdiplusToken);
		printf("failed to take the screenshot. err: %d\n", GetLastError());
		return 0;
	}

	// copy the screenshot buffer
	nCapture = SaveDC(hdcCapture);
	SelectObject(hdcCapture, hbmCapture);
	BitBlt(hdcCapture, 0, 0, w, h, dc, 0, 0, SRCCOPY);
	RestoreDC(hdcCapture, nCapture);
	DeleteDC(hdcCapture);
	DeleteDC(dc);

	// save the buffer to a file   
	pScreenShot = new Bitmap(hbmCapture, (HPALETTE)NULL);
	EncoderParameters encoderParams;
	encoderParams.Count = 1;
	encoderParams.Parameter[0].NumberOfValues = 1;
	encoderParams.Parameter[0].Guid = EncoderQuality;
	encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParams.Parameter[0].Value = &uQuality;
	GetEncoderClsid(L"image/jpeg", &imageCLSID);

	wchar_t* lpszFilename = new wchar_t[filename.length() + 1];
	mbstowcs(lpszFilename, filename.c_str(), filename.length() + 1);

	iRes = (pScreenShot->Save(lpszFilename, &imageCLSID, &encoderParams) == Ok);
	delete pScreenShot;
	DeleteObject(hbmCapture);
	GdiplusShutdown(gdiplusToken);
	return iRes;
}

void lidar(double horiFovMin, double horiFovMax, double vertFovMin, double vertFovMax, double horiStep, double vertStep, int range, std::string filePath, double error, int errorDist, std::ofstream& log)
{
	std::vector<double> dist_vector, error_vector;
	readErrorFile(dist_vector, error_vector);

	Vector3 centerDot = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0, 0, 1.2);

	int resolutionX, resolutionY;
	GRAPHICS::GET_SCREEN_RESOLUTION(&resolutionX, &resolutionY);

	//Set camera on top of player
	log << "Setting up camera...";
	Vector3 rot = CAM::GET_GAMEPLAY_CAM_ROT(0);
	Cam panoramicCam = CAM::CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", 1);
	CAM::SET_CAM_FOV(panoramicCam, 90);
	CAM::SET_CAM_ROT(panoramicCam, 0, 0, rot.z, 2);
	CAM::ATTACH_CAM_TO_ENTITY(panoramicCam, PLAYER::PLAYER_PED_ID(), 0, 0, 1.2, 1);
	CAM::RENDER_SCRIPT_CAMS(1, 0, 0, 1, 0);
	CAM::SET_CAM_ACTIVE(panoramicCam, true);
	WAIT(50);
	log << " Done.\n";

	int vertexCount = (horiFovMax - horiFovMin) * (1 / horiStep) * (vertFovMax - vertFovMin) * (1 / vertStep);
	log << "Creating dynamic array size[" + std::to_string(vertexCount) + "]...";
	Vector3* points = NULL;
	points = new Vector3[vertexCount * 1.5];
	log << " Done.\n";
	std::ofstream fileOutput, fileOutputPoints, fileOutputError, fileOutputErrorPoints, fileOutputErrorDist, fileOutputRenato;
	fileOutput.open(filePath + ".ply");
	fileOutputPoints.open(filePath + "_points.txt");
	fileOutputError.open(filePath + "_error.ply");
	fileOutputErrorPoints.open(filePath + "_error.txt");
	fileOutput << "ply\nformat ascii 1.0\nelement vertex " + std::to_string((int)vertexCount) + "\nproperty float x\nproperty float y\nproperty float z\nend_header\n";
	fileOutputError << "ply\nformat ascii 1.0\nelement vertex " + std::to_string((int)vertexCount) + "\nproperty float x\nproperty float y\nproperty float z\nend_header\n";
	//fileOutputPoints << std::to_string(centerDot.x) + " " + std::to_string(centerDot.y) + " " + std::to_string(centerDot.z) + " 0 0 0\n";
	//fileOutputError << std::to_string(centerDot.x) + " " + std::to_string(centerDot.y) + " " + std::to_string(centerDot.z) + " 0 0 0\n";

	//Disable HUD and Radar
	UI::DISPLAY_HUD(false);
	UI::DISPLAY_RADAR(false);

	float x2d, y2d, err_x2d, err_y2d;
	std::string vertexData = "", vertexDataPoints = "", vertexError = "", vertexErrorPoints = "";

	GAMEPLAY::SET_TIME_SCALE(StopSpeed);
	//Take 3D point cloud
	log << "Taking 3D point cloud...\n";
	int k = 0;
	for (double z = horiFovMin; z < horiFovMax; z += horiStep)
	{
		for (double x = vertFovMin; x < vertFovMax; x += vertStep)
		{
			ray result = angleOffsetRaycast(x, z, range);
			//Add points to the point cloud file
			vertexData += std::to_string(result.hitCoordinates.x - centerDot.x) + " " + std::to_string(result.hitCoordinates.y - centerDot.y) + " " + std::to_string(result.hitCoordinates.z - centerDot.z) + "\n";
			points[k] = result.hitCoordinates;
			k++;
		}
	}
	log << std::to_string(k) + " points filled.\nDone.\n";

	//Set clear weather
	GAMEPLAY::CLEAR_OVERRIDE_WEATHER();
	GAMEPLAY::SET_OVERRIDE_WEATHER("CLEAR");
	//Set time to midnight
	TIME::SET_CLOCK_TIME(0, 0, 0);
	WAIT(100);
	//Rotate camera 360 degrees and take screenshots
	for (int i = 0; i < 3; i++) {
		//Rotate camera
		rot.z = i * 120;
		CAM::SET_CAM_ROT(panoramicCam, 0, 0, rot.z, 2);
		WAIT(200);

		//Save screenshot
		std::string filename = filePath + "_Camera_Print_Night_" + std::to_string(i) + ".bmp";
		SaveScreenshot(filename.c_str());
	}

	//Set clear weather
	GAMEPLAY::CLEAR_OVERRIDE_WEATHER();
	GAMEPLAY::SET_OVERRIDE_WEATHER("OVERCAST");
	//Set time to noon
	TIME::SET_CLOCK_TIME(12, 0, 0);
	//Rotate camera 360 degrees and take screenshots
	WAIT(100);
	for (int i = 0; i < 3; i++) {
		//Rotate camera
		rot.z = i * 120;
		CAM::SET_CAM_ROT(panoramicCam, 0, 0, rot.z, 2);
		WAIT(200);

		//Save screenshot
		std::string filename = filePath + "_Camera_Print_Cloudy_" + std::to_string(i) + ".bmp";
		SaveScreenshot(filename.c_str());
	}

	//Set clear weather
	GAMEPLAY::CLEAR_OVERRIDE_WEATHER();
	GAMEPLAY::SET_OVERRIDE_WEATHER("CLEAR");
	//Rotate camera 360 degrees and take screenshots
	for (int i = 0; i < 3; i++) {
		//Rotate camera
		rot.z = i * 120;
		CAM::SET_CAM_ROT(panoramicCam, 0, 0, rot.z, 2);
		WAIT(200);

		//Save screenshot
		std::string filename = filePath + "_Camera_Print_Day_" + std::to_string(i) + ".bmp";
		SaveScreenshot(filename.c_str());

		log << "Converting 3D to 2D...";
		for (int j = 0; j < vertexCount; j++)
		{
			Vector3 voxel = points[j];

			//Get screen coordinates of 3D voxels
			GRAPHICS::_WORLD3D_TO_SCREEN2D(voxel.x, voxel.y, voxel.z, &x2d, &y2d);
			//Introduce error in voxels
			Vector3 xyzError;
			introduceError(&xyzError, voxel.x - centerDot.x, voxel.y - centerDot.y, voxel.z - centerDot.z, error, errorDist, range, dist_vector, error_vector);
			//Get screen coordinates of 3D voxels w/ error
			GRAPHICS::_WORLD3D_TO_SCREEN2D(xyzError.x + centerDot.x, xyzError.y + centerDot.y, xyzError.z + centerDot.z, &err_x2d, &err_y2d);

			if (x2d != -1 || y2d != -1) {
				vertexDataPoints += std::to_string(voxel.x - centerDot.x) + " " + std::to_string(voxel.y - centerDot.y) + " " + std::to_string(voxel.z - centerDot.z) + " " + std::to_string(int(x2d * resolutionX * 1.5)) + " " + std::to_string(int(y2d * resolutionY * 1.5)) + " " + std::to_string(i) + "\n";
			}
			if (err_x2d > -1 || err_y2d > -1) {
				vertexError += std::to_string(xyzError.x - centerDot.x) + " " + std::to_string(xyzError.y - centerDot.y) + " " + std::to_string(xyzError.z - centerDot.z) + "\n";
				vertexErrorPoints += std::to_string(xyzError.x - centerDot.x) + " " + std::to_string(xyzError.y - centerDot.y) + " " + std::to_string(xyzError.z - centerDot.z) + " " + std::to_string(int(err_x2d * resolutionX * 1.5)) + " " + std::to_string(int(err_y2d * resolutionY * 1.5)) + " " + std::to_string(i) + "\n";
			}
		}
		log << "Done.\n";
	}

	log << "Deleting array...";
	delete[] points;
	points = NULL;
	log << "Done.\n";
	log << "Writing to files...";
	fileOutput << vertexData;
	fileOutputPoints << vertexDataPoints;
	fileOutputError << vertexError;
	fileOutputErrorPoints << vertexErrorPoints;
	log << "Done.\n";
	GAMEPLAY::SET_GAME_PAUSED(false);
	TIME::PAUSE_CLOCK(false);
	WAIT(10);

	fileOutput.close();
	fileOutputPoints.close();
	fileOutputError.close();
	fileOutputErrorPoints.close();
	log.close();

	//Restore original camera
	CAM::RENDER_SCRIPT_CAMS(0, 0, 0, 1, 0);
	CAM::DESTROY_CAM(panoramicCam, 1);

	notificationOnLeft("LiDAR Point Cloud written to file.");
	//Unpause game
	GAMEPLAY::SET_TIME_SCALE(NormalSpeed);
	//Restore HUD and Radar
	UI::DISPLAY_RADAR(true);
	UI::DISPLAY_HUD(true);
}

void ScriptMain()
{
	srand(GetTickCount());
	while (true)
	{
		if (IsKeyJustUp(VK_F6))
		{
			try
			{
				std::ofstream log;
				log.open("LiDAR GTA V/log.txt");
				log << "Starting...\n";
				double parameters[6];
				int range;
				int errorDist;
				double error;
				std::string filename;
				std::string ignore;
				std::ifstream inputFile;
				inputFile.open("LiDAR GTA V/LiDAR GTA V.cfg");
				if (inputFile.bad()) {
					notificationOnLeft("Input file not found. Please re-install the plugin.");
					return;
				}
				log << "Reading input file...\n";
				inputFile >> ignore >> ignore >> ignore >> ignore >> ignore;
				for (int i = 0; i < 6; i++) {
					inputFile >> ignore >> ignore >> parameters[i];
				}
				inputFile >> ignore >> ignore >> range;
				inputFile >> ignore >> ignore >> filename;
				inputFile >> ignore >> ignore >> error;
				inputFile >> ignore >> ignore >> errorDist;

				inputFile.close();
				std::string newfolder = "LiDAR GTA V/" + filename;
				_mkdir(newfolder.c_str());
				log << "Starting LiDAR...\n";
				lidar(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5], range, newfolder + "/" + filename, error, errorDist, log);
				log << "SUCCESS!!!";
				log.close();
			}
			catch (std::exception& e)
			{
				notificationOnLeft(e.what());
				return;
			}
		}
		WAIT(0);
	}
}