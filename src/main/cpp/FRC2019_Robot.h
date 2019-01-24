#pragma once


class FRC2019_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
	#ifdef Robot_TesterCode
	virtual void BindAdditionalEventControls(bool Bind,GG_Framework::Base::EventMap *em,IEvent::HandlerList &ehl)=0;
	#endif
};

struct FRC2019_Robot_Props
{
public:
	//TODO evaluate how this translates into the dart
	//everything in meters and radians
	double OptimalAngleUp;
	double OptimalAngleDn;
	double ArmLength;
	double ArmToGearRatio;
	double PotentiometerToArmRatio;
	double PotentiometerMaxRotation;
	double GearHeightOffset;
	double MotorToWheelGearRatio;
	struct Autonomous_Properties
	{
		size_t AutonTest;
		//void ShowAutonParameters(); //This will show SmartDashboard variables if ShowParameters is true
		bool ShowParameters;   //If true ShowAutonParameters will populate SmartDashboard with autonomous parameters
	} Autonomous_Props;
};

class FRC2019_Robot_Properties : public Tank_Robot_Properties
{
	public:
		FRC2019_Robot_Properties();
		virtual void LoadFromScript(Scripting::Script& script);
		
		const Rotary_Pot_Properties &GetArmProps() const {return m_ArmProps;}
		const Rotary_Properties &GetClawProps() const {return m_ClawProps;}
		const FRC2019_Robot_Props &GetFRC2019RobotProps() const {return m_FRC2019RobotProps;}
		FRC2019_Robot_Props &GetFRC2019RobotProps_rw() { return m_FRC2019RobotProps; }
		const LUA_Controls_Properties &Get_RobotControls() const {return m_RobotControls;}
	private:
		#ifndef _Win32
		typedef Tank_Robot_Properties __super;
		#endif

		Rotary_Pot_Properties m_ArmProps;
		Rotary_Properties m_ClawProps;
		FRC2019_Robot_Props m_FRC2019RobotProps;

		class ControlEvents : public LUA_Controls_Properties_Interface
		{
			protected: //from LUA_Controls_Properties_Interface
				virtual const char *LUA_Controls_GetEvents(size_t index) const; 
		};
		static ControlEvents s_ControlsEvents;
		LUA_Controls_Properties m_RobotControls;

};


const char * const csz_FRC2019_Robot_SpeedControllerDevices_Enum[] =
{
	"arm","roller"
};

//Note: rotary systems share the same index as their speed controller counterpart
const char * const csz_FRC2019_Robot_AnalogInputs_Enum[] =
{
	"arm_pot"
};

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class FRC2019_Robot : public Tank_Robot
{
	public:
		enum SolenoidDevices
		{
			eDeployment,
			eClaw,
			eRist
		};
		enum SpeedControllerDevices
		{
			eArm,eRoller
		};
		static SpeedControllerDevices GetSpeedControllerDevices_Enum(const char *value)
		{
			return Enum_GetValue<SpeedControllerDevices>(value, csz_FRC2019_Robot_SpeedControllerDevices_Enum, _countof(csz_FRC2019_Robot_SpeedControllerDevices_Enum));
		}
		enum AnalogInputs
		{
			eArmPot
		};
		static AnalogInputs GetAnalogInputs_Enum(const char *value)
		{
			return Enum_GetValue<AnalogInputs>(value, csz_FRC2019_Robot_AnalogInputs_Enum, _countof(csz_FRC2019_Robot_AnalogInputs_Enum));
		}


		FRC2019_Robot(const char EntityName[],FRC2019_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);
		void CloseDeploymentDoor(bool Close);

		const FRC2019_Robot_Properties &GetRobotProps() const;
		FRC2019_Robot_Props::Autonomous_Properties &GetAutonProps();

		//TODO test roller using is angular to be true
		class Robot_Claw : public Rotary_Velocity_Control
		{
			public:
				Robot_Claw(FRC2019_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//public access needed for goals
				void CloseClaw(bool Close);
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on);
				void Squirt(bool on);
			protected:
				//Intercept the time change to send out voltage
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			private:
				#ifndef _Win32
				typedef Rotary_Velocity_Control __super;
				#endif
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				FRC2019_Robot * const m_pParent;
				bool m_Grip,m_Squirt;
		};
		class Robot_Arm : public Rotary_Position_Control
		{
			public:
				Robot_Arm(FRC2019_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//The parent needs to call initialize
				double GetPosRest();
				void CloseRist(bool Close);
			protected:
				//Intercept the time change to obtain current height as well as sending out the desired velocity
				virtual void BindAdditionalEventControls(bool Bind);
				void Advance(bool on);
				void Retract(bool on);
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);

			private:
				#ifndef _Win32
				typedef Rotary_Position_Control __super;
				#endif
				FRC2019_Robot * const m_pParent;
				bool m_Advance, m_Retract;
		};

		//Accessors needed for setting goals
		Robot_Arm &GetArm() {return m_Arm;}
		Robot_Claw &GetClaw() {return m_Claw;}
		//Allow external code to switch the default goal
		void SetTestAutonCallbackGoal(std::function<Goal *(FRC2019_Robot *Robot)> callback) { m_TestAutonGoalCallback = callback; }
	protected:
		virtual void BindAdditionalEventControls(bool Bind);
		virtual void BindAdditionalUIControls(bool Bind, void *joy, void *key);
	private:
		FRC2019_Robot_Properties m_RobotProps;
		#ifndef _Win32
		typedef  Tank_Robot __super;
		#endif
		FRC2019_Control_Interface * const m_RobotControl;
		Robot_Arm m_Arm;
		Robot_Claw m_Claw;
		bool m_VoltageOverride;  //when true will kill voltage
		std::function<Goal *(FRC2019_Robot *Robot)> m_TestAutonGoalCallback=nullptr;  //pointer to function to obtain goal to use for TestAutonomous to use
		//No longer are these restricted to simulation
		void TestAutonomous();
		void StopAuton(bool isOn);
		void GoalComplete();
		bool m_SmartDashboard_AutonTest_Valve = false; //Value used to detect change of AutonTest CheckBox
};


namespace FRC2019_Goals
{
	Goal *Get_FRC2019_Autonomous(FRC2019_Robot *Robot);
	//Temporary for testing
	#if 1
	Goal *Get_Sample_Goal(FRC2019_Robot *Robot);
	#endif
}

//TODO enable this
#if 1
///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class FRC2019_Robot_Control : public frc::RobotControlCommon, public FRC2019_Control_Interface
{
	public:
		FRC2019_Robot_Control(bool UseSafety = true);
		virtual ~FRC2019_Robot_Control();
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		FRC2019_Control_Interface &AsControlInterface() { return *this; }
		const FRC2019_Robot_Properties &GetRobotProps() const { return m_RobotProps; }
		//Give access to set hooks in the drive as well
		void SetDriveExternalPWMSpeedControllerHook(std::function<void *(size_t, size_t, const char *, const char*)> callback) 
		{
			#if !defined _Win32 || defined __Tank_TestControlAssignments__
			m_TankRobotControl.SetExternalPWMSpeedControllerHook(callback);
			#endif
		}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index, double Voltage);
		virtual void CloseSolenoid(size_t index,bool Close);
		virtual void OpenSolenoid(size_t index,bool Open) {CloseSolenoid(index,!Open);}
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		//Unfortunately the actual wheels are reversed (resolved here since this is this specific robot)
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(RightVoltage,LeftVoltage);}
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		//pacify this by returning its current value
		virtual double GetRotaryCurrentPorV(size_t index);
		virtual void UpdateRotaryVoltage(size_t index, double Voltage) { UpdateVoltage(index, Voltage); }

		//These should no longer be necessary
		//virtual void CloseRist(bool Close) {CloseSolenoid(FRC2019_Robot::eRist,Close);}
		//virtual void OpenRist(bool Close) {CloseSolenoid(FRC2019_Robot::eRist,!Close);}
		protected: //from RobotControlCommon
			virtual size_t RobotControlCommon_Get_PWMSpeedController_EnumValue(const char *name) const
			{	return FRC2019_Robot::GetSpeedControllerDevices_Enum(name);
			}
			virtual size_t RobotControlCommon_Get_DigitalInput_EnumValue(const char *name) const
			{  //return FRC2019_Robot::GetBoolSensorDevices_Enum(name);
				return 0;
			}
			virtual size_t RobotControlCommon_Get_AnalogInput_EnumValue(const char *name) const
			{	return FRC2019_Robot::GetAnalogInputs_Enum(name);
			}
			virtual size_t RobotControlCommon_Get_DoubleSolenoid_EnumValue(const char *name) const
			{	//return Curivator_Robot::GetSolenoidDevices_Enum(name);
				return 0;
			}

	protected: //from FRC2019_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);

	protected:
		FRC2019_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		frc::Compressor *m_Compressor=nullptr;
		//frc::Accelerometer *m_RoboRIO_Accelerometer=nullptr;   <---for reference
		//Base::EventMap* m_EventMap=nullptr;  <---TODO see if we need this

		double m_ArmMaxSpeed;
		//cache voltage values for display   -Depreciated
		//double m_ArmVoltage,m_RollerVoltage;
		bool m_Deployment,m_Claw,m_Rist;
	private:
		//Note: these may be arrayed if we have more pots
		KalmanFilter m_KalFilter_Arm;
		Averager<double, 5> m_Averager;
		#ifdef _Win32
		Potentiometer_Tester3 m_Potentiometer; //simulate a real potentiometer for calibration testing
		#endif
		bool m_FirstRun = false;
};
#endif

#ifdef Robot_TesterCode

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC2019_Robot_UI : public FRC2019_Robot, public FRC2019_Robot_Control
{
	public:
		FRC2019_Robot_UI(const char EntityName[]) : FRC2019_Robot(EntityName,this),FRC2019_Robot_Control(),
			m_TankUI(this) {}
	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_TankUI.TimeChange(dTime_s);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_TankUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_TankUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
			{m_TankUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_TankUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_TankUI.UpdateScene(geode,AddOrRemove);}

	private:
		Tank_Robot_UI m_TankUI;

};

#endif

