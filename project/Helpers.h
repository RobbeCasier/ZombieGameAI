#pragma once
#include "Exam_HelperStructs.h"
struct House
{
	HouseInfo houseInfo;
	bool explored = false;
};

//vector2 operators
bool operator>(Elite::Vector2 a, Elite::Vector2 b)
{
	return a.x > b.x && a.y > b.y;
}

bool operator<(Elite::Vector2 a, Elite::Vector2 b)
{
	return a.x < b.x && a.y < b.y;
}