#define API_VERSION "0.04"
/**

 @brief Acc-Mag-Sensor API version 0.04 2/19/2021 last edited by PJS

 ==== Notes about the changes from 0.03 to 0.04
 This reflects our phone conversation. The earlier notion of fine grain 
 sensitivity is morphed into a new setRange() function. Function getAxisData()
 is added to return a struct of six int16_t values containing the current
 measurements. We also realized a new  basic requirement for a polled mode in
 addition to the currently defind interrupt-driven type of operation. Polled
 mode is necessary to deal with the availability of pins to dedicate to
 interrupt connections. The convention will be that if an event callback
 argument to the configuration class constructor is NULL polling mode will be
 implemented. With polling new bool functions getAccEvent() and getMagEvent() 
 will be in the API, returning true if an event has been detected, with the 
 function call clearing the event state. The return value when not in polling
 mode will always be false.

 But specific pin assignments will still be defined by platform-specific,
 compile-time assignments in the implemention using a helper header file or a
 library. We should probably call this something like siHAL for See Insights 
 Hardware Abstraction L{ayer,ibrary}. This would anticipate additional
 systems needing the HAL vs tainting this API's library with details that
 should be more general.

 ===== The API design notes and declarations

 This library is designed to provide a basic, initial implementation with a
 single policy governing acceleration and magnetometer sensor operation with 
 both one fixed response sensor type as well as self-recalibration behavior 
 for specific circumstances with the magnetic sensor. It is also designed to 
 also allowing for flexible extension of policies of behavior and the setting 
 of a range of refined or new configuration parameters as field tests expose 
 additional requirements.

 When an acceleration event is detected a user defined callback function is 
 invoked (called), or, if none is defined, internal state is changed and that
 state can be fetched with a getter function. A second user defined callback 
 function is invoked each time a magnetic field event is detected, or else
 corresponding state is available when using polling. The details of what 
 constitutes an event for each sensor are defined by policies and any specific 
 configuration settings made prior to instantiating the control object for the 
 sensor. There are two configuration-related setter functions per sensor
 type: one each for setting the sensor range and the sensor sensitivity.

 Configuration details are contained in a separate configuration object that
 is passed as a parameter to the control object class construtor. Once the
 control object is instantiated the available API functions will be to do
 with gathering details of operation in addition to having the two callback
 actions: no dynamic adjustment of policy or configuration parameters or
 other "knobs" are available once the control object is created. There will
 be a getter function for retrieving the current values for all six sensor
 axes. 

 With the recent change to have all sensor handling remote from the sensor
 everything to do with the sensor becomes configurable by over the air
 firmware updates. The API design assumes this flexibility.

 The goal is to allow for incremental adjustments and improvements as actual
 field testing expose both common and uncommon operating conditions.

 The default policy for the library will define triggering of exceeding 
 thresholds of z axis values for either sensor. 

 If the single axis policy is inadequate new policies can be defined
 in terms of, for example, different axis thresholds, multiple interrupt 
 handling states, combinations of multiple axis value detection sequences, etc.

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

 HOWEVER, the "freeze" of a policy definition is only completed when the API
 implementation has been promoted to production status (commited to the
 production workspace in the repository).
  
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
  AMS_CONFIG_NOT_VALID_AFTER_BEGIN,	// Can't call config after sys start
  AMS_NOT_IMPLEMENTED,			// Not (yet) implemented
  AMS_ILLEGAL_SENSITIVITY,		// Illegal sensitivity value
  AMS_ILLEGAL_RANGE,		        // Illegal range value
  AMS_IMPROPER_ORIENTATION,		// System failure: not placed properly
  AMS_NOT_RUNNING,			// Cannot end() stopped system
  AMS_RUNNING,				// Cannot begin running system
  AMS_WHOAMI_FAILURE}; 			// System failure: sensor not found

typedef enum AMSStatusEnum AMSStatus;

struct AMSData_struct {
  int16_t a_x, a_y, a_z, m_x, m_y, m_z;
};

typedef struct AMSData_struct AMSData;

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
     * @param accEvent - func called with acceleration event or NULL
     * @param magEvent - func called with magnetic field event or NULL
     * @param policy - the enumeration value of the desired sensor system behavior policy
     * @param logical_device - an ordinal specifying a single device (placeholder)
     */

    AccMagSensorConfig( void (*accEvent)(uint32_t device), 
		void (*magEvent)(uint32_t device),
                Policy policy = default_policy,
                uint32_t logical_device = 0);

    /**
     * @brief Override the default accelerometer sensor sensitivity
     * 
     * @param sensitivity_value - the desired sensitivity from 0-9
     * @return The OK or exception status value
     */

    AMSStatus setAccSensitivity(uint32_t sensitivity_value);

    /**
     * @brief Override the default magnetometer sensor sensitivity
     * 
     * @param sensitivity_value - the desired sensitivity from 0-9
     * @return The OK or exception status value
     */

    AMSStatus setMagSensitivity(uint32_t sensitivity_value);

    /**
     * @brief override the default accelerometer sensor range
     * 
     * @param range_value - the sensor range value
     * @return The OK or exception status value
     */

    AMSStatus setAccRange(int16_t range_value);

    /**
     * @brief override the default magnetometer sensor range
     * 
     * @param range_value - the sensor range value
     * @return The OK or exception status value
     */

    AMSStatus setMagRange(int16_t range_value);

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
   * @brief get the current acceleration event state
   * @detail always returns false if not in polling mode
   * @return true if an event is present, false otherwise
   */

  bool getAccEvent();

  /**
   * @brief get the current magnetometer event state
   * @detail always returns false if not in polling mode
   * @return true if an event is present, false otherwise
   */

  bool getMagEvent();

  /**
   * @brief fill the fields of the structure parameter with current axis values
   * @param axis_data
   */

  AMSStatus getAxisData(AMSData &axis_data);

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
