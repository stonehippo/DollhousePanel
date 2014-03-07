# Dollhouse Lighting Control System

This is the logic for a dollhouse lighting control system that I am building for my daughter.

## The Boards

This code is designed to drive an Arduino-compatible board with a several components attached. For my build, I'm using an Arduino clone from SmartMaker*, along with a few other boards from that system. Here's what I've got going:

* Smartmaker Smartcore U–basically, a weird Arduino Uno)
* A Smartmaker Smartbus Quad–this provides the base for the whole system, as well as the power regulator for the Smartcore
* Smartmaker Breakout–a simple breakout board for the microcontroller pins
* Smartmaker LCD 16x2–a standard 16x2 LCD display
* Smartmaker RGB LED board–a single RGB LED
* Smartmaker Button 5A5–an array of 5 momentary switches, attached to analog pins on the Smartcore
* Smartmaker AB Riser–a little board to give some stuff a lift!
* Adafruint 12 channel PWM board-this is a sweet little board that gives you 12 channels for driving LEDs at 16-bit resolution
* Some Adafruit breadboard-friendly NeoPixels
* RadioShack white LED strip–this was originally a single 60 LED, 1 meter strip, which I cut up to make a bunch of 3 LED strips (one for each room)

## The Pins!

One of the more annoying things about the Smartmaker stuff is a poor planning for the pin usage for the various components. For example, because the 16x2 LCD is wired up directly, rather than with a serial backpack or something, the board requires 6 of the Arduino's precious digital I/O pins. After factoring in the other components, I am using in almost all of the pins on the board.

Here's how things map out:

<table>
	<tbody>
		<tr>
			<th>Compnent</th>
			<th>Pins used</th>
		</tr>
		<tr>
			<td>RGB LED</td>
			<td>9,10,11</td>
		</tr>
		<tr>
			<td>LCD</td>
			<td>2,3,4,5,6,7</td>
		</tr>
		<tr>
			<td>TL59711 PWM Board</td>
			<td>12,13</td>
		</tr>
		<tr>
			<td>NeoPixels</td>
			<td>8</td>
		</tr>
		<tr>
			<td>Buttons</td>
			<td>A0,A1,A2,A3,A4</td>
		</tr>
	</tbody>
</table>

----------

\* I acquired the Smartmaker stuff as a backer reward from their less-than-stellar Kickstarter campaign. Expect for the components that I have used in this project, I have sold off all of my Smartmaker Open System components.

