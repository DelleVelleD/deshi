#pragma once
#ifndef DESHI_TIME_H
#define DESHI_TIME_H

#include "../utils/defines.h"

#include <chrono>
#include <time.h>
#include <ctime>

struct Time{
	float  prevDeltaTime;
	float  deltaTime;
	double totalTime;
	u64 updateCount;
	
	float  fixedTimeStep;
	float  fixedDeltaTime;
	double fixedTotalTime;
	u64 fixedUpdateCount;
	float  fixedAccumulator;
	
	bool paused, frame;
	
	std::time_t end_time;
	char datentime[30] = {};

	std::time_t now;

	std::tm* ltm;

	int year;
	int month;
	int day;
	std::string weekday;//probably should use like char array or something idk

	int hour;
	int minute;
	int second;



	
	std::chrono::time_point<std::chrono::system_clock> tp1, tp2;
	
	void Init(float fixedUpdatesPerSecond){
		prevDeltaTime = 0;
		deltaTime     = 0;
		totalTime     = 0;
		updateCount   = 0;
		
		fixedTimeStep    = fixedUpdatesPerSecond;
		fixedDeltaTime   = 1.f / fixedUpdatesPerSecond;
		fixedTotalTime   = 0;
		fixedUpdateCount = 0;
		fixedAccumulator = 0;
		
		paused = false;
		frame  = false;
		
		tp1 = std::chrono::system_clock::now();
		tp2 = std::chrono::system_clock::now();
	}
	
	void Update(){
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		
		prevDeltaTime = deltaTime;
		deltaTime = elapsedTime.count();
		
		end_time = std::chrono::system_clock::to_time_t(tp2);
		ctime_s(datentime, sizeof(datentime), &end_time);
		
		if(!paused){
			totalTime += deltaTime;
			++updateCount;
			fixedAccumulator += deltaTime;
		}else if(frame){
			totalTime += deltaTime;
			++updateCount;
			fixedAccumulator += deltaTime;
			frame = false;
		}else{
			deltaTime = 0;
		}

		//TODO(Op, sushi) make it so things that dont update frequently like date stuff only updates when the time changes over to the next day n stuff 
		now = time(0);

		ltm = localtime(&now);

		year = ltm->tm_year + 1900;
		month = ltm->tm_mon + 1;
		day = ltm->tm_mday;

		hour = ltm->tm_hour + 1;
		minute = ltm->tm_min;
		second = ltm->tm_sec;

		switch (ltm->tm_wday) {
		case 0:
			weekday = "Mon";
			break;
		case 1:
			weekday = "Tue";
			break;
		case 2:
			weekday = "Wed";
			break;
		case 3:
			weekday = "Thu";
			break;
		case 4:
			weekday = "Fri";
			break;
		case 5:
			weekday = "Sat";
			break;
		case 6:
			weekday = "Sun";
			break;
		}
	}
};

#endif //DESHI_TIME_H