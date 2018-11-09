#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>
#include <time.h>

#include "glm/glm.hpp"
#include "glm/detail/type_vec3.hpp""

int RayIntersect(glm::vec3 &_vecA, glm::vec3 &_vecB, glm::vec3 &_vecC, glm::vec3 &_vecOrig, glm::vec3 &_vecDir, float *_fRay, glm::vec3 &_vecE1, glm::vec3 &_vecE2, bool &_bCull);
void LoadObj(std::string &_filename, std::vector<glm::vec3> &_rawPositionData, std::vector<glm::ivec3> &_posIndices);

int main()
{
	//Declare varaibles
	std::string sFilePath;
	std::vector<glm::vec3> vVertexData;
	std::vector<glm::ivec3> vFaceVertexID;
	float fRay[256];
	bool bExit = false;
	std::string sExit;
	bool bCull = false;

	std::cout << "\nEnter y key to enable culling of backfacing triangles. Enter any other key to continue." << std::endl;
	std::getline(std::cin, sExit);
	if (sExit == "y")
	{
		bCull = true;
	}

	//Initialise ray array
	for (int i = 0; i < 256; i++)
	{
		fRay[i] = i * 1.0f;
	}

	//Initialise vec3s
	glm::vec3 vecA, vecB, vecC;
	glm::vec3 vecOrig = { 0.0, 0.0, 0.0 };
	glm::vec3 vecDir = { 1.0, 0.0, 0.0 };

	while (bExit == false)
	{
		//Get .obj and initialise data vectors
		std::cout << "Enter file path:" << std::endl;
		std::getline(std::cin, sFilePath);
		LoadObj(sFilePath, vVertexData, vFaceVertexID);

		//Declare and initialise recording variables
		int iCount = 0;	//Counts number of intersects
		clock_t cTime;	//Measures time taken to complete ray-triangle intersection
		cTime = clock();

		//Begin triangle loop
		for (std::vector<glm::ivec3>::iterator it = vFaceVertexID.begin(); it != vFaceVertexID.end(); it++)
		{
			//Set triangle vertices
			vecA = vVertexData.at(it->x);
			vecB = vVertexData.at(it->y);
			vecC = vVertexData.at(it->z);

			//E1/2 are edges of the triangle. Nor is the plane normal created from cross of two edges and normalised by dividing by the magnitude
			glm::vec3 vecE1 = vecB - vecA, vecE2 = vecC - vecA;

			iCount = iCount + RayIntersect(vecA, vecB, vecC, vecOrig, vecDir, fRay, vecE1, vecE2, bCull);
		}

		//Display results
		cTime = clock() - cTime;
		std::cout << "Time in seconds: " << ((float)cTime) / CLOCKS_PER_SEC << std::endl;
		std::cout << "Number of intersects: " << iCount << std::endl;

		//Exit program or begin another ray-triangle intersection test with new .obj
		std::cout << "\nEnter x key to exit program or enter any other key to test another .obj" << std::endl;
		std::getline(std::cin, sExit);
		if (sExit == "x")
		{
			bExit = true;
		}
	}
}

int RayIntersect(glm::vec3 &_vecA, glm::vec3 &_vecB, glm::vec3 &_vecC, glm::vec3 &_vecOrig, glm::vec3 &_vecDir, float *_fRay, glm::vec3 &_vecE1, glm::vec3 &_vecE2, bool &_bCull)
{
	//Set up intersect counter
	int iCount = 0;

	//Begin ray loop
	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			//Set ray
			_vecOrig.y = _fRay[i];
			_vecOrig.z = _fRay[j];

			//Get and check determinant
			glm::vec3 vecP = glm::cross(_vecDir, _vecE2);
			float fDet = glm::dot(_vecE1, vecP);

			//Check determinant
			if (_bCull == true)
			{
				if (fDet < std::numeric_limits<double>::epsilon())
				{
					//no intersect
					continue;
				}
			}
			else
			{
				if (fDet < std::numeric_limits<double>::epsilon() && fDet > -std::numeric_limits<double>::epsilon())
				{
					//no intersect
					continue;
				}
			}

			float fInverse = 1 / fDet;

			//Get and check U value
			glm::vec3 vecT = _vecOrig - _vecA;
			float fUValue = glm::dot(vecT, vecP) * fInverse;
			if (fUValue < 0 || fUValue > 1)
			{
				//does not intersect
				continue;
			}

			//Get and check V value
			glm::vec3 vecQ = glm::cross(vecT, _vecE1);
			float fVValue = glm::dot(vecQ, _vecDir) * fInverse;
			if (fVValue < 0 || fUValue + fVValue > 1)
			{
				//does not intersect
				continue;
			}

			//T parameter of ray equation
			float fTIntersect = glm::dot(vecQ, _vecE2) * fInverse;
			//Intersection point
			glm::vec3 vecIntersect = _vecOrig + (fTIntersect * _vecDir);

			//Display the ray intersect
			std::cout << "Ray origin: " << _vecOrig.x << "," << _vecOrig.y << "," << _vecOrig.z << " Ray Direction: " <<
			_vecDir.x << "," << _vecDir.y << "," << _vecDir.z << " intersects with triangle A:" << _vecA.x << "," <<
			_vecA.y << "," << _vecA.z << " B:" << _vecB.x << "," << _vecB.y << "," << _vecB.z << " C:" << _vecC.x <<
			"," << _vecC.y << "," << _vecC.z << " at: " << vecIntersect.x << "," << vecIntersect.y << "," << vecIntersect.z << std::endl;
			iCount++;
		}
	}
	return iCount;
}

void LoadObj(std::string &_sFilename, std::vector<glm::vec3> &_rawPositionData, std::vector<glm::ivec3> &_posIndices)
{
	std::ifstream fObject(_sFilename);
	_rawPositionData.clear();
	_posIndices.clear();

	if (fObject.is_open())
	{
		std::string currentLine;

		while (std::getline(fObject, currentLine))
		{
			std::stringstream currentLineStream(currentLine);

			if (!currentLine.substr(0, 2).compare(0, 2, "vt"))
			{
				//do nothing
			}
			else if (!currentLine.substr(0, 2).compare(0, 2, "vn"))
			{
				//do nothing
			}
			else if (!currentLine.substr(0, 2).compare(0, 1, "v"))
			{
				//do nothing
				std::string junk;
				float x, y, z;
				currentLineStream >> junk >> x >> y >> z;
				_rawPositionData.emplace_back(glm::vec3(x, y, z));
			}
			else if (!currentLine.substr(0, 2).compare(0, 1, "f"))
			{
				std::string junk;
				std::string verts[4];
				int x, y, z;

				currentLineStream >> junk >> verts[0] >> verts[1] >> verts[2] >> verts[3];

				if (verts[3].empty())
				{
					for (unsigned int i = 0; i < 3; i++)
					{
						std::stringstream currentSection(verts[i]);

						// There is just position data
						unsigned int posID = 0;

						if (verts[i].find('/') == std::string::npos)
						{
							// No texcoords or normals
							currentSection >> posID;
						}
						else if (verts[i].find("//") != std::string::npos)
						{
							// No texcoords
							char junk;
							currentSection >> posID >> junk >> junk >> junk;
						}
						else
						{
							char junk;
							currentSection >> posID >> junk >> junk >> junk >> junk;
						}

						if (posID > 0)
						{
							switch (i)
							{
								case 0:
								{
									x = posID - 1;
									break;
								}
								case 1:
								{
									y = posID - 1;
									break;
								}
								case 2:
								{
									z = posID - 1;
									_posIndices.emplace_back(glm::vec3(x, y, z));
									break;
								}
								default: {break; }
							}
						}

					}
				}
				else
				{
					std::cerr << "WARNING: This OBJ loader only works with triangles but a quad has been detected. Please triangulate your mesh." << std::endl;
					fObject.close();
					return;
				}

			}
		}

		fObject.close();

	}
	else
	{
		std::cerr << "WARNING: File not found: " << _sFilename << std::endl;
	}
}
