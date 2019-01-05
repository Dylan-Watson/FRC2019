/****************************** Header ******************************\
Class Name: ActiveCollection
File Name:	ActiveCollection.cpp
Summary: Stores all Components on the robot controlled by the software.
Project:     BroncBotzFRC2019
Copyright (c) BroncBotz.
All rights reserved.

Author(s): Ryan Cooper
Email: cooper.ryan@centaurisoftware.co
\********************************************************************/

#include <iostream>

#include "ActiveCollection.h"

using namespace std;
using namespace System;

/**
 * Default Constructor
**/
ActiveCollection::ActiveCollection(){}

/**
 * Method to return a NativeComponent of a certain type and name
**/
//TODO: Does this even work?
template <class T>
T ActiveCollection::Get(string name){
	try{
		for(int i=0; i<(int)activeCollection.size();i++)
			if((*activeCollection[i]).name == (string)name)
				return &activeCollection[i];
		throw "AHHH";
	}
	catch(...){
		cout << "Cannot find component " << name << ", it does not exist in the active collection!\nCheck your type correlation!" << endl;
	}
}

/**
 * Method to return a VictorSP of a certain name
**/
//TODO: change all current instances of Victor to VictorSP as we will be adding Victor SPX very soon 
VictorSPItem* ActiveCollection::GetVictor(string name){
	try{
		for(int i=0; i<(int)activeCollection.size();i++){
			if((*activeCollection[i]).name == (string)name){
				return (VictorSPItem*)activeCollection[i];
			}
		}
		throw "AHHH";
	}
	catch(...){
		cout << "Cannot find victor " << name << ", it does not exist in the active collection!\n" << endl;
	}
}

/**
 * Method to return the current size of the ActiveCollection list
**/
int ActiveCollection::GetSize(){
	return activeCollection.size();
}

/**
 * Adds a component to the ActiveCollection
**/
void ActiveCollection::Add(NativeComponent *component){
	try{
		activeCollection.push_back(component);
	}
	catch(...){
		cout << "Cannot add component " << (*component).name << endl;
	}
}

vector<NativeComponent*> ActiveCollection::GetRawComponent(){
	return activeCollection;
}