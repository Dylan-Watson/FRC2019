/****************************** Header ******************************\
Class Name: (multiple classes)
File Name: FRC2019_Goals.cpp
Summary: All implemented Goals go here. Region statements categorize
types
Project: BroncBotzFRC2019
Copyright (c) BroncBotz.
All rights reserved.

Author(s): Chris Weeks
Email: chrisrweeks@aol.com
\********************************************************************/
#include "FRC2019_Goals.h"

using namespace std;

//?HINT ctrl+k then ctrl+0 will collapse all regions
//?ctrl+k then ctrl+j will uncollapse all regions





#pragma region AtomicGoals
#pragma region TimerGoals
/***********************Goal_Wait_ac***********************/
void Goal_Wait_ac::Activate()
{
    m_Status = eActive;
}
Goal::Goal_Status Goal_Wait_ac::Process(double dTime)
{
    if (m_Status == eActive)
    {
        m_currentTime += dTime;
        if (m_currentTime >= m_timeOut)
        {
            Terminate();
            return m_Status = eCompleted;
        }

        return eActive;
    }
    else
    {
        return m_Status;
    }
}

void Goal_Wait_ac::Terminate()
{
}
/***********************Goal_TimeOut***********************/

Goal::Goal_Status Goal_TimeOut::Process(double dTime)
{
    if (m_Status == eActive)
    {
        Log::General("timeOutProcess");
        m_currentTime += dTime;
        SmartDashboard::PutNumber("timeOut",m_currentTime);
        if (m_currentTime >= m_timeOut)
        {
            Terminate();
            return m_Status = eFailed;
        }
        //cout << m_activeCollection->GetNavX()->GetAngle() << endl; //DEBUG
        return eActive;
    }
    else
    {
        return m_Status;
    }
}
void Goal_TimeOut::Terminate()
{
}
/***********************Goal_DriveWithTimer***********************/
Goal::Goal_Status Goal_DriveWithTimer::Process(double dTime)
{
    Log::General("b4");
    if (m_Status == eActive)
    {
        Log::General("processing");
        m_currentTime += dTime;
        SetDrive(m_leftSpeed, m_rightSpeed, m_activeCollection);
        if (m_currentTime >= m_timeOut)
        {

            Terminate();
            return m_Status = eCompleted;
        }
        return m_Status = eActive;
    }
    else
    {
        return m_Status;
    }
}

void Goal_DriveWithTimer::Terminate()
{
    StopDrive(m_activeCollection);
}
#pragma endregion

#pragma region ControllerOverride

void Goal_ControllerOverride::Activate()
{
    m_Status = eActive;
}

Goal::Goal_Status Goal_ControllerOverride::Process(double dTime)
{
    if(eActive)
    {
		if (m_controller == 0)
			TestDriver();
		else if (m_controller == 1)
			TestOperator();
        else
        {
			TestDriver();
			if (m_Status==eActive)
				TestOperator();
        }
    }
	return m_Status;
}

void Goal_ControllerOverride::Terminate()
{
    SetCallbacks(false);
}

void Goal_ControllerOverride::TestDriver()
{
    if (m_IsDriveInUse)
	{
		DriverValueChanged(new SenderEventArgs<Goal_ControllerOverride*>(this));  
		m_Status = eFailed;
	}
}

void Goal_ControllerOverride::TestOperator()
{
	if (m_IsOperatorInUse)
	{
		OperatorValueChanged(new SenderEventArgs<Goal_ControllerOverride*>(this));
		m_Status = eFailed;
	}
}

void Goal_ControllerOverride::SetCallbacks(bool bind)
{
    auto onValueChanged = [&](EventArgs* e) {
        try{
            auto args = (TEventArgs<double, Controls::ControlItem*>*)e;
            if(args->GetSender()->joy->GetPort() == 0){
                m_IsDriveInUse = true;
            }
            if(args->GetSender()->joy->GetPort() == 1){
                m_IsOperatorInUse = true;
            }
        }catch(exception &e){
            Log::Error("Known Exception Thrown in onValueChanged in a ControllerOverride! This can cause fatal Runtime Errors! Check your logs and XML.");
            Log::Error(e.what());
        }catch(...){
            Log::Error("UnknownException Thrown in onValueChanged in ControllerOverride! This can cause fatal Runtime Errors! Check your XML and yell at the programmers!");
	    }
    };

    if(bind){
        for(Event *e : m_activeCollection->EventMap)
            (*e).subscribeFirstPriority(onValueChanged);
    }else{
        for(Event *e : m_activeCollection->EventMap)
            (*e).unsubscribeFirstPriority();
    }
}
#pragma endregion

void Goal_ElevatorControl::Activate()
{
    m_Status = eActive;
    error = 0;
    deriv = 0;
    integ = 0;
}

#if 0
Goal::Goal_Status Goal_ElevatorControl::Process(double dTime)
{
    if(m_Status == eActive)
    {
        m_timeElapsed += dTime;
        if(((m_pot->Get() > m_target && m_goingUp) || (m_pot->Get() < m_target && !m_goingUp)))
        {
            Terminate();
            return m_Status = eCompleted;
        }
        else
        {
            //look at elevator trapezoid in design folder
            if(m_goingUp)
            {
                if(m_timeElapsed < .25)
                {
                    SetElevator((MAX_POWER-MIN_POWER)/.25 * m_timeElapsed + MIN_POWER, m_activeCollection);
                }
                else if(m_timeElapsed > .25 && m_timeElapsed < moveTime)
                {
                    SetElevator(MAX_POWER, m_activeCollection);
                }
                else if(m_timeElapsed > moveTime && m_timeElapsed < moveTime + .75)
                {
                    SetElevator(((MIN_POWER-MAX_POWER)/.25)*(m_timeElapsed - moveTime - .25) + MAX_POWER, m_activeCollection);
                }
                else
                {
                    Terminate();
                    return m_Status = eCompleted;
                }
                
                
            }
            else
            {
                //exact same math as above but with a negative in front
                if(m_timeElapsed < .25)
                {
                    SetElevator(-((MAX_POWER-MIN_POWER)/.25 * m_timeElapsed + MIN_POWER), m_activeCollection);
                }
                else if(m_timeElapsed > .25 && m_timeElapsed < moveTime)
                {
                    SetElevator(-MAX_POWER, m_activeCollection);
                }
                else if(m_timeElapsed > moveTime && m_timeElapsed < moveTime + .25)
                {
                    SetElevator(-(((MIN_POWER-MAX_POWER)/.25)*(m_timeElapsed - moveTime - .25) + MAX_POWER), m_activeCollection);
                }
                else
                {
                    Terminate();
                    return m_Status = eCompleted;
                }
            }
        }
    }
    else
    {
        return m_Status;
    }
    
}
#else 
Goal::Goal_Status Goal_ElevatorControl::Process(double dTime)
{
    if(m_Status == eActive)
    {
        Log::General("ele target: " + to_string(m_target));
#ifndef _Win32
        m_currentPos = m_pot->Get();
        #else 
        m_currentPos = .6;
        #endif
        if(m_currentPos >= m_target - FREEDOM && m_currentPos <= m_target + FREEDOM)
        {
            Terminate();
            nt::NetworkTableInstance::GetDefault().GetTable("DASHBOARD_TABLE")->PutNumber("pot",m_currentPos);
            return eCompleted;
        }
        error = m_target - m_currentPos;
        integ += error * dTime;
        deriv = (error - errorPrior) / dTime;

        double power = kp * error + ki * integ + kd * deriv;
        if(power > MAX_POWER) power = MAX_POWER;
        if(power < -MAX_POWER) power = -MAX_POWER;
        SetElevator(power, m_activeCollection);
        return eActive;

    }
    else
    {
        return m_Status;
    }
    
}
#endif

void Goal_ElevatorControl::Terminate()
{
    StopElevator(m_activeCollection);
}

void Goal_RelativeElevatorControl::Activate()
{
    goal->Activate();
    m_Status = eActive;
}

Goal::Goal_Status Goal_RelativeElevatorControl::Process(double dTime)
{
    Goal::Goal_Status stat = goal->Process(dTime);
    if(stat == eCompleted)
        ((DoubleSolenoidItem*)(m_activeColelction->Get("hatch_push")))->DefaultSet();
    return stat;
}

void Goal_RelativeElevatorControl::Terminate()
{
    goal->Terminate();
}
#pragma region FeedbackLoopGoals
/***********************Goal_Turn***********************/
void Goal_Turn::Activate()
{
    Goal_Wait_ac::Activate();
    m_navx->Reset();
}

Goal::Goal_Status Goal_Turn::Process(double dTime)
{
    if (m_Status == eActive)
    {
        m_currentTime += dTime;
        if (m_currentTime > m_timeOut)
        {
            Terminate();
            //cout << "no target" << endl;
            //return m_Status = eFailed; //set m_Status to failed and return m_Status in one line
        }

        double lowerBound = m_target - m_freedom;
        double upperBound = m_target + m_freedom;
        double currentAngle = m_navx->GetAngle();
        //cout << "ANGLE: " << currentAngle << endl;

        propError = (m_target - currentAngle) / m_target;
        integ += propError * dTime;               //Right Riemann Sum integral
        deriv = (propError - errorPrior) / dTime; // rise/run slope
        errorPrior = propError;                   //set errorPrior for next process call

        m_power = bias + (kp * propError) + (ki * integ) + (kd * deriv); //power is equal to P,I,D * k-values + bias

        if (currentAngle < lowerBound || currentAngle > upperBound)
        {
            SetDrive(m_power, -m_power, m_activeCollection);
            return m_Status = eActive;
        }
        else
        {
            Terminate();
            return m_Status = eCompleted;
        }
    }
    else
    {
        return m_Status;
    }
}

void Goal_Turn::Terminate()
{
    StopDrive(m_activeCollection);
}

/***********************Goal_DriveStraight***********************/
void Goal_DriveStraight::Activate()
{
    Goal_Wait_ac::Activate();
	if (m_encLeft)
	{
		m_encLeft->Reset();
		m_encRight->Reset();
	}
	else
		m_Status = Goal::eFailed;
}

//Left enc = master, right enc = slave
Goal::Goal_Status Goal_DriveStraight::Process(double dTime)
{
    if (m_Status = eActive)
    {
		SmartDashboard::PutBoolean("DRIVE STRAIGHT STATUS", true);
        m_currentTime += dTime;
        if (m_currentTime > m_timeOut)
            return m_Status = eFailed;
        m_distTraveled += m_encLeft->Get();
        if (m_distTraveled >= m_distTarget)
        {
            Terminate();
            return m_Status = eCompleted;
        }

        //TODO PID

        error = m_encLeft->Get() - m_encRight->Get(); //we cannot use true propError here because target is zero.
        integ += error * dTime;                       //Right Riemann Sum integral
        deriv = (error - errorPrior) / dTime;         // rise/run slope
        errorPrior = error;                           //set errorPrior for next process call

        //left power never changes
        m_rightPower += bias + (error * kp) + (integ * ki) + (deriv * kd);
        SetDrive(m_leftPower, m_rightPower, m_activeCollection);

        m_encLeft->Reset();
        m_encRight->Reset();
    }
    else
    {
		SmartDashboard::PutBoolean("DRIVE STRAIGHT STATUS", true);
        return m_Status;
    }
}

void Goal_DriveStraight::Terminate()
{
    StopDrive(m_activeCollection);
}
/***********************Goal_VisionAlign***********************/
void Goal_VisionAlign::Activate()
{
    Log::General("vision goal active");
    m_Status = eActive;
    updateVision();
}

Goal::Goal_Status Goal_VisionAlign::Process(double dTime)
{
    m_currentTime += dTime;
    if(m_currentTime > m_timeOut)
    {
        Log::General("time out");
        Terminate();
        return m_Status = eFailed;
    }
    updateVision();
    Log::General( to_string(m_currentTarget->getX()) + " " + to_string(m_currentTarget->getY()) + " " + to_string(m_currentTarget->getRadius()) + " " + to_string(Height) + " " + to_string(Width) + to_string(HasTarget));
    if(!HasTarget) 
    {
        Log::General("no target");
        StopDrive(m_activeCollection);
        return m_Status = eActive; //!return failed for real thing or search
    }
    if(m_target->compareX(m_currentTarget) < -20) 
    {
        Log::General("turn left");
        SetDrive(m_target->compareX(m_currentTarget) * TURN_KP,-(m_target->compareX(m_currentTarget) * TURN_KP),m_activeCollection);
    }
    else if(m_target->compareX(m_currentTarget) > 20)
    {
        Log::General("turn right");
        SetDrive(m_target->compareX(m_currentTarget) * TURN_KP,-(m_target->compareX(m_currentTarget) * TURN_KP),m_activeCollection);
    }
    else
    {
        if(m_target->compareRadius(m_currentTarget) < -2 || m_target->compareRadius(m_currentTarget) > 2)
        {
            Log::General("drive");
            SetDrive(m_target->compareRadius(m_currentTarget) * STRAIGHT_KP,(m_target->compareRadius(m_currentTarget) * STRAIGHT_KP),m_activeCollection);
        }
        else
        {
            Log::General("aligned");
            Terminate();
            //return m_Status = eCompleted;
        }
        
    }
    //cout << "nothing" << endl;
    return m_Status = eActive;

}

void Goal_VisionAlign::Terminate()
{
    StopDrive(m_activeCollection);
}

#pragma endregion

#pragma region UtilGoals
#if 0
/***********************Goal_Hatch***********************/
void Goal_Hatch::Activate()
{
    m_Status = eActive;
}
#if 0
Goal::Goal_Status Goal_Hatch::Process(double dTime)
{
    //TODO: Yeet on this
}
void Goal_Hatch::Terminate()
{
	//TODO: Yeet o this
}
#endif
#endif
#pragma endregion
#pragma endregion

#pragma region CompositeGoals
/***********************Goal_WaitThenDrive***********************/
void Goal_WaitThenDrive::Activate()
{
    AddSubgoal(new Goal_Wait_ac(m_activeCollection, m_waitTime));
    AddSubgoal(new Goal_DriveWithTimer(m_activeCollection, m_leftSpeed, m_rightSpeed, m_driveTime));

    m_Status = eActive;
}

/***********************Goal_OneHatch***********************/
void Goal_OneHatchFrontShip::Activate()
{
    m_Status = eActive;
    if(m_position == "Level 1 Left")
    {
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(5.0), .75));
        AddSubgoal(new Goal_Turn(m_activeCollection, 90));
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(3.0), .75));
        AddSubgoal(new Goal_Turn(m_activeCollection, -90));
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(2.0), .5));
        //GOAL DEPLOY HATCH
    }
    else if(m_position == "Level 1 Center")
    {
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(7.0), .75));
        //deploy hatch
    }
    else if(m_position == "Level 1 Right")
    {
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(5.0), .75));
        AddSubgoal(new Goal_Turn(m_activeCollection, -90));
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(3.0), .75));
        AddSubgoal(new Goal_Turn(m_activeCollection, 90));
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(2.0), .5));
        //GOAL DEPLOY HATCH
    }
    else if(m_position == "Level 2 Left")
    {
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(5.0), .75));
        AddSubgoal(new Goal_OneHatchFrontShip(m_activeCollection, "Level 1 Left"));
    }
    else if(m_position == "Level 2 Right")
    {
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(5.0), .75));
        AddSubgoal(new Goal_OneHatchFrontShip(m_activeCollection, "Level 1 Right"));
    }
    else
    {
        AddSubgoal(new Goal_DriveStraight(m_activeCollection, new Feet(10.0), .75));
    }
    
}
#pragma endregion

#pragma region MultitaskGoals

#pragma endregion