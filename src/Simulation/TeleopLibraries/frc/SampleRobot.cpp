#include <frc/WPILib.h>
#include <frc/SampleRobot.h>

using namespace frc;

bool RobotBase::IsEnabled() const
{
	return false;
}

bool RobotBase::IsDisabled() const
{
	return false;
}

bool RobotBase::IsAutonomous() const
{
	return false;
}

bool RobotBase::IsOperatorControl() const
{
	return false;
}


bool RobotBase::IsTest() const
{
	return false;
}
