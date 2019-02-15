/****************************** Header ******************************\
Class Name: Robot inherits SampleRobot
File Name: Robot.cpp
Summary: Entry point from WPIlib, main class where robot routines are
started.
Project: BroncBotzFRC2019
Copyright (c) BroncBotz.
All rights reserved.

Author(s): Ryan Cooper, Dylan Watson, Chris Weeks
Email: cooper.ryan@centaurisoftware.co, dylantrwatson@gmail.com, 
chrisrweeks@aol.com
\********************************************************************/

#include <iostream>
#include "Robot.h"
//TODO: Potentiometer include, VictorSPX include
using namespace std;

/**
 * Constructor
 */
Robot::Robot() 
{
	m_drive = new Drive();
	m_activeCollection = new ActiveCollection();
}

Robot::~Robot()
{
	delete m_drive;
	m_drive = nullptr;
	delete m_activeCollection;
	m_activeCollection = nullptr;
}

/*
 * Initialization method of the robot
 * This runs right after Robot() when the code is started
 * Creates Config
 */
void Robot::RobotInit()
{
	Log::restartfile();
	cout << "Program Version: " << VERSION << " Revision: " << REVISION << endl;
	//CameraServer::GetInstance()->StartAutomaticCapture(0);
	Config *config = new Config(m_activeCollection, m_drive); //!< Pointer to the configuration file of the robot
	//Must have this for smartdashboard to work properly
	SmartDashboard::init();
	//Select which lua to load here
	#ifdef _Win32
	const char *RobotScript = "FRC2019_Simulation.lua";
	//const char *RobotScript = "FRC2019Robot.lua";
	#else
	const char *RobotScript = "FRC2019Robot.lua";
	#endif	
	m_Robot.RobotAssem_init(RobotScript, m_activeCollection);
	m_inst = nt::NetworkTableInstance::GetDefault();		  //!Network tables
	m_visionTable = m_inst.GetTable("VISION_2019");			  //!Vision network table
	m_dashboardTable = m_inst.GetTable("DASHBOARD_TABLE");
	m_dashboardTable->PutStringArray("AUTON_OPTIONS", m_autonOptions);
	m_dashboardTable->PutStringArray("POSITION_OPTIONS", m_positionOptions);


	//TODO: put this in some sort of config
	m_visionTable->PutNumber("LS",0);
	m_visionTable->PutNumber("US",3);
	m_visionTable->PutNumber("LH",87);
	m_visionTable->PutNumber("UH",126);
	m_visionTable->PutNumber("LV",255);
	m_visionTable->PutNumber("UV",255);
	m_visionTable->PutNumber("MinA",1112);
	m_visionTable->PutNumber("MaxA",82763);
	m_visionTable->PutNumber("MaxO",62);
}

/*
 * Method that runs at the beginning of the autonomous mode
 * Uses the SmartDashboard to select the proper Autonomous mode to run
  TODO: Integrate with our dashboad; Integrate with TeleOp for sandstorm; make everything that should be abstract is in fact abstract; Integrate vision overlay and drive cam to dashboard; Write teleop goals;
 */

void Robot::Autonomous()
{


	m_masterGoal = new MultitaskGoal_ac(m_activeCollection, false);

	cout << "Autonomous Started." << endl;
	//string autoSelected = SmartDashboard::GetString("Auto Selector", m_driveStraight);
	string autoSelected = m_dashboardTable->GetString("AUTON_SELECTION", m_driveStraight);
	string positionSelected = m_dashboardTable->GetString("POSITION_SELECTION", "NONE"); //if it is none, then just drive straight
	cout << autoSelected << endl;
	if (!SelectAuton(m_activeCollection, m_masterGoal, autoSelected, positionSelected))
	{
		m_dashboardTable->PutString("AUTON_FOUND", "UNDEFINED AUTON OR POSITION SELECTED");
	}
	//m_masterGoal->AddGoal(new Goal_TimeOut(m_activeCollection, 15.0)); //!Disabled for testing!!
	//m_masterGoal->AddGoal(new Goal_WaitThenDrive(m_activeCollection, .5, .5, 3, 5));
	m_masterGoal->Activate();
	double dTime = 0.010;
	while (m_masterGoal->GetStatus() == Goal::eActive && _IsAutononomous() && !IsDisabled())
	{
		m_masterGoal->Process(dTime);
		Wait(dTime);
	}
	m_masterGoal->~MultitaskGoal_ac();
	cout << "goal loop complete" << endl;
	cout << m_activeCollection->GetNavX()->GetAngle() << endl;
#if 0 //old code
		if (autoSelected == m_driveStraight) //!< Drive Straight Autonomous
		{
			DriveStraight *driveStraight = new DriveStraight(m_activeCollection);
			driveStraight->Start();
		}

		else //!< Default Autonomous
		{
			DriveStraight *driveStraight = new DriveStraight(m_activeCollection);
			driveStraight->Start();
		}
#endif
}

/*
 * Called when teleop starts
 */
void Robot::OperatorControl()
{
	#if 0
	cout << "Teleoperation Started." << endl;
	while (IsOperatorControl() && !IsDisabled())
	{
		m_drive->Update();
		Wait(0.010);
	}
	#else
	Log::restartfile();
	cout << "Teleoperation Started." << endl;
	double LastTime = GetTime();
	//We can test teleop auton goals here a bit later
	while (IsOperatorControl() && !IsDisabled())
	{
		const double CurrentTime = GetTime();
		const double DeltaTime = CurrentTime - LastTime;
		LastTime = CurrentTime;
		if (DeltaTime == 0.0) continue;  //never send 0 time
		//printf("DeltaTime=%.2f\n",DeltaTime);
		#ifndef _Win32
		m_Robot.Update(DeltaTime);
		#else
		m_Robot.Update(0.01);  //It's best to use sythetic time for simulation to step through code
		#endif
		//m_drive->Update();
		//using this from test runs from robo wranglers code
		Wait(0.010);
	}
	#endif
}

/*
 * Called when the Test period starts
 */
void Robot::Test()
{
}

START_ROBOT_CLASS(Robot) //!< This identifies Robot as the main Robot starting class
