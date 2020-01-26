#include <dreammakerfx.h>

/**
 * Multitudes -> Put some delay in your delay!
 *
 * OVERVIEW:
 * ========
 *
 * A variable length delay pedal where the feedback path of the main delay is routed to a second delay.
 *
 * CONTROLS:
 * ========
 *
 * Left pot: feedback setting on driver delay.
 * 
 * Center pot: delay time on driver delay.
 * 
 * Right pot: propagation delay ratio.
 *
 *    Controls the delay time of the propagation delay as a function of the driver delay time.
 *    
 *    If the knob is set to noon (set to halfway):
 *      The Propagation delay will be of the same length as the driver delay
 *      
 *    If the knob is set between 6pm and noon (set between 0-5):
 *      The Propagation delay will be *shorter* than the driver delay. 
 *      Shortest possible propagator delay length is 1/4 the time of the driver delay. 
 *
 *    If the knob is set between noon and 6pm (set between 5-10):
 *      The Propagation delay will be *longer* than the driver delay.
 *      Longest possible propagation delay length is driver delay time * 4.
 *
 * Left footswitch: bypass - turns on and off the effect
 * 
 * Right footswitch: tap - tap it a few times at a set interval to update the delay rate
 * 
 */

const float max_delay_ms = 3000.0;
const float max_prop_factor = 4.0;
const float max_prop_ms = max_delay_ms * max_prop_factor;

fx_delay driver(
    1000.0,         // Initial delay length of 1 second / 1000ms
    max_delay_ms,   // Max delay
    0.5,            // Feedback value
    1.0,            // Clean mix
    0.7,            // Delay / echo mix
    true            // Enable send/receive
);        

fx_delay propagator(
    1000.0,         // Initial delay length of 1 second / 1000ms
    max_prop_ms,    // Max delay 
    0.4,            // feedback value
    1.0,            // Clean mix
    0.5,            // Delay / echo mix
    false           // Disable send/receive
); 


void setup() {

  pedal.init();
  
  // Route audio through effects
  pedal.route_audio(pedal.instr_in, driver.input);
  pedal.route_audio(driver.output, pedal.amp_out); 
  
  // Route filter through delay fxsend/receive loop
  pedal.route_audio(driver.fx_send, propagator.input);
  pedal.route_audio(propagator.output, driver.fx_receive);

  // Use left footswitch/LED to bypass effect
  pedal.add_bypass_button(FOOTSWITCH_LEFT); 

  // Right foot switch is tap delay length
  pedal.add_tap_interval_button(FOOTSWITCH_RIGHT, true);
  
  pedal.run(); 

}

float get_driver_len() {
  // Helper function to get current driver delay length in ms
  
  return 100.0 + pedal.pot_center.val*(max_delay_ms-100.0);

} 

void update_driver() {  
  // Helper function to set the driver delay length in ms

  float new_driver_len = get_driver_len();
  driver.set_length_ms(new_driver_len);
  pedal.set_tap_blink_rate_ms(new_driver_len);

}

void update_propagator(float driver_delay_len) {
  // Helper function to set delay length on the propagator delay.

  /*
   * propagator delay time is a function of two values:
   *  
   *  1.) The delay time currently set on the driver delay
   *  2.) The position of the Propagation Delay Ratio knob
   * 
   *  If the Propagation Delay Ratio knob is between noon and 6pm:
   *    The Propagation delay knob position is mapped to prop_ratio: a value between [1, 4]
   *    The Propagation delay time is set to: (Driver Delay Time * Prop Ratio) 
   * 
   *  If the Propagation Delay Ratio knob is between noon and 6pm:
   *    The Propagation delay knob position is mapped to prop_ratio: a value between [4, 1] (note the direction change)
   *    The Propagation delay time is set to: (Driver Delay Time / Prop Ratio) 
   * 
   *  If the Propagation Delay Ratio knob is set to noon: 
   *    The Propagation delay is the same as the Driver Delay length.
   */
  
  float prop_ratio, new_prop_len;
    
  if (pedal.pot_right.val > 0.5) {
    prop_ratio = map(pedal.pot_right.val, .5, 1, 1, int(max_prop_factor));
    new_prop_len = driver_delay_len * prop_ratio;
  } else if (pedal.pot_right.val < 0.5) {
    prop_ratio = map(pedal.pot_right.val, 0, .5, int(max_prop_factor), 1);
    new_prop_len = driver_delay_len / prop_ratio;
  } else {
    new_prop_len = driver_delay_len;
  }
   
  propagator.set_length_ms(new_prop_len);

}

void loop() {
  
  if (pedal.pot_left.has_changed()) {
    driver.set_feedback(pedal.pot_left.val);
  }

  if (pedal.pot_center.has_changed() || pedal.pot_right.has_changed()) {
    update_driver();
    update_propagator(get_driver_len());
  }    

  if (pedal.new_tap_interval()) { 
    float new_tap_len = pedal.get_tap_interval_ms();
    driver.set_length_ms(new_tap_len);
    pedal.set_tap_blink_rate_ms(new_tap_len);
    update_propagator(new_tap_len);
  } 
  
  pedal.service(); 

}
