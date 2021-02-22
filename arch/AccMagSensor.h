#define API_VERSION "0.03"
/**

 @brief Acc-Mag-Sensor API version 0.03 2/16/2021 last edited by PJS

   THIS NAME SEEMS TOO SPECIFIC. SHOULD IT BE "See-Insights-Sensor"? 

 and please ignore indentation foul ups for now :-)

 ==== Notes about the changes from 0.02 to 0.03
 CHIP: This reflects our phone conversation to the best of my knowledge. I
 have to ask you to read this top to bottom again. Hopefully with the next
 update a "diff" will be more useful.

 One type of change will be from realizing better what needs to be hidden in 
 the API's implementation and what sort of interfaces need to be exposed in 
 the public part of the API. In particular, details like interrupt polarity 
 would be in the former category, while the course 1-10 and finer grain 1-100 
 sensitivity setting as well as the means for "not properly oriented" 
 indication need to be accessible to the Particle apps using the API with
 a pragmatic design. You also need as much explanation as possible for what
 the sensitivity numbers translate to with the sensor, such as milligravities
 that you an relate to IC datasheet details. I believe we also agreed that the 
 details of pin usage can be automagically handled by the API implementation
 but at the same time the details should be here so the user of the API 
 doesn't accidentally oversubscribe a pin. For our currently anticipated 
 hardware I think we agreed that the default "SS" pin for each of the flavors
 of Particle board will be used for chip select, so I should be able to
 define all of this without any ambiguity EXCEPT for the interrupt lines.
 There are one to three of these, depending on the IC involved, but typically
 only two will be used. These will have to be listed with agreed to values
 along with the "standard" pins for the communications bus interface (e.g.
 SPI).

 We can't really afford to expose ANY hardware configuration specifics beyond
 those inherent to accelerometers and magnetometers via the public API without 
 getting into two kinds of trouble:
  1) We switch to some new system that requires, for example, a new flavor
     of interrupt type that doesn't fit into push-pull+active high, etc.
  2) We want the compilation of the API implementation to detect and set
     hardware specifics via build configuration variables. So specifying
     details in two places is trouble.

 The usage.ino is split out into an examples subdirectory (if you don't mind
 I'm going to use "directory" instead of "folders", know you know the
 equivalence). If I've got the dir or filenames a bit messed up be patient
 with me. I'll get it sorted after I've been able to restructure the repo.
 I realized that while we have active branches in addition to main I need
 to sanity check the restructuring plan with you before pulling the trigger.
 Details about that can come a little later.

 ===== The API design notes and declarations

 This library is designed to provide a basic, initial implementation with a
 single policy governing acceleration and magnetometer sensor operation with 
 both one fixed response sensor type as well as self-recalibration behavior 
 for specific circumstances with the magnetic sensor. It is also designed to 
 also allowing for flexible extension of policies of behavior and the setting 
 of a range of refined or new configuration parameters as field tests expose 
 additional requirements.

 When an acceleration event is detected a user defined callback function is 
 invoked (called), and a second user defined callback function is invoked each 
 time a magnetic field event is detected. The details of what constitutes an
 event for each sensor are defined by policies and any specific configuration
 settings made prior to instantiating the control object for the sensor. 
 Configuration details are contained in a separate configuration object that
 is passed as a parameter to the control object class construtor. Once the
 control object is instantiated the available API functions will be to do
 with gathering details of operation in addition to having the two callback
 actions: no dynamic adjustment of policy or configuration parameters or
 other "knobs" are available once the control object is created. The
 function calls for providing information, for example current axis threshold
 values, "history" of past events or other state information are TBD. 

(CHIP: I'm expecting you to make a list of these "monitor" function calls
  for the next update). 

 With the recent change to have all sensor handling remote from the sensor
 everything to do with the sensor becomes configurable by over the air
 firmware updates. The API design assumes this flexibility.

 The goal is to allow for incremental adjustments and improvements as actual
 field testing expose both common and uncommon operating conditions.

 The default policy for the library will define triggering of exceeding 
 thresholds of z axis values for either sensor. 

 One "sensitivity knob" function for each sensor will be available via the 
 config object for control of the sensor response to accelerations and changes
 in magnetic field strength respectively. Sensitivity can be set to an ordinal
 value from zero to nine.

 At least one getter function for each sensor type will be available to 
 translate a source sensitivity setting into sensor-specific details such as
 microgravity threshold, microtesla threshold, etc. There may be additional
 getters to express the sensitivity such as with additional axis values being
 involved. These getters may be tightly coupled to policy names. That is,
 the initial getter call to "tell me the Z axis mg corresponding to sensitivity
 setting five" be to getDefaultSensitvity(5), while for another policy foo it
 will have to be getFooSensitivity(5). The rationale for this is discussed
 below to do with maintaining backward compatibility.

 If the single axis policy is inadequate new policies can be defined
 in terms of either different axis thresholds, or possibly using multiple
 interrupt handling states, combinations of multiple axis value detection
 sequences. 

 The expectation is that the design/test/enhance flow steps will
 proceed something like this:
  1) Field test
  2) Change of compile time configuration values and/or changes of runtime
     coarse sensitivity value
  3) goto 1.

 If at some point step (2) above is found to be inadequate one or more of the
 following steps are anticipated:
  1) Creation of a new policy. 
  2) Implement the changes of sensor settings and runtime handling of sensor
     behavior as needed and possibly add new API elements such as function 
     calls and/or additional function parameters.

 Existing policies should *never be modified*, ever, and backwards
 compatibility should be maintained: a "big rule". The reason for this big
 rule is that until the sensor system matures it is expected that there will
 be many circumstances in which "reality" is hard to see. The ability to
 reproduce prior results must not be sabotaged by backwards-incompatible 
 changes to the API (architecture). Implied by this requirement is the need 
 to very carefully handle addition of function parameters (deletion is 
 forbidden) and avoid changing the semantic defintion of existing parameters.

 With modern tool chains there is nearly zero overhead for unused functions
 and the same holds for inactive policies. Policy-specific code (not shared
 with the implementation of active polices) can be omitted from a build if
 this becomes important.

 Regression tests should detect violations of the big rule.

 HOWEVER, it may be deemed necessary to declare the initial API and its
 implementation with an "initial" default policy as "so broken it's not even
 wrong", in which case by agreement a new default policy is define and that
 starts the "clock" and the no violation of backward compatibility rule.
  
 The AccMagSensorConfig class defines a configuration object with an initial
 state depending on its class constructor parameters and any setter functions
 called before it is passed to the AccMagSensor class constructor. 

 The AccMagSensor class constructor takes a single parameter: the above
 configuration object. Having two classes allows for support for multiple
 sensors as well as sensors having differing configurations in the future.

 The expected use of the callback functions is setting a volatile bool flag
 or changing some other state very simply. Any function calls within the
 callback must avoid interactions with the underlying OS state that could 
 cause undesirable behavior (for instance an infinitely long wait for some
 other interrupt that is blocked by the callback).  

 See examples/usage.ino for an example of expected API use.

*/

/**
 * @brief The sensor system status definitions
 */

enum AMSStatusEnum {
  AMS_OK,                           	// Normal completion
  AMS_NOT_IMPLEMENTED,			// Not (yet) implemented
  AMS_ILLEGAL_SENSITIVITY,		// Illegal sensitivity value
  AMS_ILLEGAL_THRESHOLD,		// Illegal threshold value
  AMS_IMPROPER_ORIENTATION,		// System failure: not placed properly
  AMS_NOT_RUNNING,			// Cannot end() stopped system
  AMS_RUNNING,				// Cannot begin running system
  AMS_WHOAMI_FAILURE}; 			// System failure: sensor not found

typedef enum AMSStatusEnum AMSStatus;

/*
 * The API class declarations
 *
 */


/**
 * @brief AccMagSensorConfig class for configuration of the sensor system
 */

class AccMagSensorConfig {

  public:

    enum AccMagSensorPolicyValue {default_policy};  // placeholder for future
    typedef enum AccMagSensorPolicyValue Policy;

    /**
     * @brief The AccMagSensorConfig class constructor
     *
     * @detail Invalid parameters will result in a false return from AccMagSensor.begin (how to make doxygen link here?)
     * 
     * @param accEvent - func called with acceleration event (no default)
     * @param magEvent - func called with magnetic field event (no default)
     * @param policy - the enumeration value of the desired sensor system behavior policy
     * @param logical_device - an ordinal specifying a single device (placeholder)
     */

    AccMagSensorConfig( void (*accEvent)(uint32_t device), 
		void (*magEvent)(uint32_t device),
                Policy policy = default_policy,
                uint32_t logical_device = 0);

    /**
     * @brief A configuration function to specify the desired sensor sensitivity
     * 
     * @param interrupt_type - the desired interrupt output type
     * @return The OK or exception status value
     */

    AMSStatus setSensitivity(uint32_t sensitivity_value);

    /**
     * @brief set the specific Z axis accel threshold for "minimum sensitivity"
     * 
     * @param threshold - the positive or negative axis threhold value
     * @return The OK or exception status value
     */

    AMSStatus setAzMinThreshold(int16_t threshold);

    /**
     * @brief set the specific Z axis accel threshold for "maximum sensitivity"
     * 
     * @param threshold - the positive or negative axis threhold value
     * @return The OK or exception status value
     */

    AMSStatus setAzMaxThreshold(int16_t threshold);

    /**
     * @brief set the specific Z axis mag threshold for "minimum sensitivity"
     * 
     * @param threshold - the positive or negative axis threhold value
     * @return The OK or exception status value
     */

    AMSStatus setMzMinThreshold(int16_t threshold);

    /**
     * @brief set the specific Z axis mag threshold for "maximum sensitivity"
     * 
     * @param threshold - the positive or negative axis threhold value
     * @return The OK or exception status value
     */

    AMSStatus setMzMaxThreshold(int16_t threshold);

  /**
   * @brief the config API version
   * @return the version as a text string constant
   */

  const char *getVersionString(return API_VERSION);

  /**
   * @brief the config API version
   * @return the version as an integer: (uint32_t) (version * 100.0)
   */

  uint32_t getVersionNumber(); // e.g. "0.03" returns "3", "1.42" returns "142"

  private:
    AMSStatus *last_status = AMS_OK;
    void *s;
};// class AccMagSensorConfig

/**
 * @brief AccMagSensor class for control of the sensor system
 */

class AccMagSensor {
  public:

    /**
     * @brief the class constructor
     * @detail any problems detected in the config object will manifest as an error return from the begin function
     *
     * @param the sensor system configuration object
     */

   AccMagSensor(AccMagSensorConfig config);

   /**
    * @brief Begin sensor system operation
    * 
    * @return AMS_OK or an exception status
    */

   AMS_Status begin();

   /**
    * @brief ENd sensor system operation
    * 
    * @return AMS_OK or an exception status 
    */

   AMS_Status end();

   /**
    * @brief Get the current sensor temperature
    * 
    * @return the current sensor temperature as a signed centigrade value
    */

   int8_t getSensorTemperature();

   /**
    * @brief Perform a reset of the sensor
    * 
    * @details The details are sensor IC specific and may be a null operation.
    */
   void resetSensor();

   /**
    * @brief Determine sensor orientation
    * 
    * @return AMS_OK if orientation is in bounds, else AMS_IMPROPER_ORIENTATION
   */

   AMS_Status getOrientation();


  /**
   * @brief return the most recent sensor system status
   * @detail last_status is updated by the system asynchronously
   * @return the most recent status
   */

  AMSStatus getStatus() { return last_status; }

  /**
   * @brief the control API version
   * @return the version as a text string constant
   */

  const char *getVersionString(return API_VERSION);

  /**
   * @brief the control API version
   * @return the version as an integer: (uint32_t) (version * 100.0)
   */

  uint32_t getVersionNumber(); // e.g. "0.03" returns "3", "1.42" returns "142"

 private:
   void *s;
   AMSStatus last_status = AMS_OK;
}; // class AccMagSensor
