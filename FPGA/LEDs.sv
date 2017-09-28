// This is the current setup for which LED on the array is of which type
// The indexes on the columns indicate which rgb group it corresponds to
// The indexes on the rows indicate which enable group it corresponds to
// 
// P = varies with signal power
// X = flashes on the beat
// 2 = flashes at 1/2x the beat
// 
//   1 2 3 4 4 3 2 1
// 1 P P P P P P P P
// 2 P P P X X P P P
// 3 P P X 2 2 X P P
// 4 P X 2 X X 2 X P
// 4 P X 2 X X 2 X P
// 3 P P X 2 2 X P P
// 2 P P P X X P P P
// 1 P P P P P P P P


module arrayDriver (input logic [7:0] red, green, blue, sigPower,
						input logic clk, isBeat,
						output logic [11:0]rgbSig,
						//output logic [2:0] testLEDs,
						output logic [3:0] enables);
	// Hill Balliet - wballiet@g.hmc.edu - 11/19/16
	// This module brings all of the logic together for driving an 8x8 LED array
	// in the pattern shown above with the given color
	//
	// rgbSig is composed of:
	// {r18, g18, b18, r27, g27, b27, r36, g36, b36, r45, g45, b45}
	//
	// enables is composed of:
	// {EN18, EN27, EN36, EN45}
	
	logic [23:0] color18, color27, color36, color45, colorPTest, colorBTest, colorTwoTest;
	logic [16:0] counter;
	
	// Init to 0 for simulation
	initial
		counter = 0;

    assign colors = {red, green, blue};
		
	setColors color1(colors, sigPower, counter[16:15], 2'b00, isBeat, clk, color18);
	setColors color2(colors, sigPower, counter[16:15], 2'b01, isBeat, clk, color27);
	setColors color3(colors, sigPower, counter[16:15], 2'b10, isBeat, clk, color36);
	setColors color4(colors, sigPower, counter[16:15], 2'b11, isBeat, clk, color45);
	
	// For debugging
	//setColors pTest({8'hFF, 16'b0}, sigPower, 2'b00, 2'b00, isBeat, clk, colorPTest);
	//setColors bTest({8'hFF, 16'b0}, sigPower, 2'b11, 2'b11, isBeat, clk, colorBTest);
	//setColors twoTest({8'hFF, 16'b0}, sigPower, 2'b10, 2'b11, isBeat, clk, colorTwoTest);

	pwm rgb18(color18, clk, rgbSig[11:9]);
	pwm rgb27(color27, clk, rgbSig[8:6]);
	pwm rgb36(color36, clk, rgbSig[5:3]);
	pwm rgb45(color45, clk, rgbSig[2:0]);
	
	//assign testLEDs = {colorPTest[23], colorBTest[23], colorTwoTest[23]};
	
	
	// Always have exactly one group of enables on and all other groups off
	// If two rows are equal, their enables are grouped
	// Since we are using PNP transistors, grounding the base corresponds to saturating the 
	// transistor, so the enable bits are flipped
	always_comb
		case (counter[16:15])
			2'b00: enables = ~4'b1000; // Rows 1 and 8
			2'b01: enables = ~4'b0100; // Rows 2 and 7
			2'b10: enables = ~4'b0010; // Rows 3 and 6
			2'b11: enables = ~4'b0001; // Rows 4 and 5
		endcase
	
	
	// Count clock cycles to reduce the 40 MHz clk to ~152Hz
	always_ff @(posedge clk)
		counter++;
	
endmodule

module pwm(input logic [23:0] digColors,
			input logic clk,
			output logic [2:0] pwmColors);
	// Hill Balliet - wballiet@g.hmc.edu - 11/21/16
	// This module converts the digital representations of colors to a pulse
	// width modulation "analog" signal.  It linearly scales the digital
	// representation between the minimum on voltage for the LED and maximum
	// output voltage of 3.3V.

	logic [7:0] rmin, gmin, bmin, rchange, gchange, bchange;
	logic [7:0] rvolt, gvolt, bvolt;
	logic [15:0] rvoltlong, gvoltlong, bvoltlong;
	logic [11:0] counter, toDutyCycle;
	logic [19:0] ronTime, gonTime, bonTime;
	

	// Init to 0 for simulation
	initial
		counter = 0;
	
	//assign rmin = 8'b01_100001; // minimum on red voltage will be 1.515625V
	//assign gmin = 8'b01_111011; // minimum on green voltage will be 1.921875V
	//assign bmin = 8'b10_010011; // minimum on blue voltage will be 2.296875V

	assign rmin = 8'h0;
	assign gmin = 8'h0;
	assign bmin = 8'h0;
	
	// These are hard coded rather than found by subtraction so that leading
	// zeros do not mess up the floating point multiplication
	// slope should be (max-min)/255
	// Maximums are calibrated to show white when all colors are maxed
	// rmax = 2.50V = 10_100000
	// gmax = 3.08V = 11_000101
	// bmax = 3.19V = 11_001100
	//assign rchange = 8'b0_1111110; // slope on red voltage will be 0.984375V/255
	//assign gchange = 8'b1_0010100; // slope on green voltage will be 1.15625V/255
	//assign bchange = 8'b0_1110010; // slope on blue voltage will be 0.890625V/255
	assign rchange = 8'b10_100000;
	assign gchange = 8'b11_000101;
	assign bchange = 8'b11_001100;

	// which is the scaling factor to get from V to on time
	assign toDutyCycle = 12'b010011011000; // 4095/3.3 = 1240

	// Calculate the correct voltage required to create the color
	// Assuming the brightness of the LED varies linearly within the range where
	// it is on, the slope will be (max-min)/255 and the offset will be min.
	//
	// 1/255 is approx 0.000000001 in binary so has been simplified out of the 
	// calculations to save space
	//
	// the slope ({8'h00, (max-min)/255}) will be of the form 16'b00.000000xxxxxxxx
	// the color is of the form 16'h00XX
	// Therefore slope * color is of the form 16'hXX.xxxxxxxxxxxxxx
	// 
	// the offset (min) is of the form 8'bXX.xxxxxx
	always_comb begin
		rvoltlong = {8'h00, rchange} * {8'h00, digColors[23:16]} + {rmin, 8'h00};
		gvoltlong = ({8'h00, gchange} * {8'h00, digColors[15:8]}) + {gmin, 8'h00};
		bvoltlong = ({8'h00, bchange} * {8'h00, digColors[7:0]}) + {bmin, 8'h00};
	end
	
	assign rvolt = rvoltlong[15:8];
	assign gvolt = gvoltlong[15:8];
	assign bvolt = bvoltlong[15:8];
					
	// Convert the desired voltage to a duty cycle by determining the ratio
	// of the desired voltage to the maximum voltage.
	// The duty cycle is calculated as V/3.3*4095
	//
	// voltage is in the form 20'b000000000000XX.xxxxxx
	// toDutyCycle is 20'h00XXX
	// their product is in the form 18'bXXXX XXXX XXXX.xxxxxx
	// Because the max value of voltage is bounded by 3.3, the product will not overflow
	always_comb begin
		ronTime = {12'b0, rvolt} * {8'b0, toDutyCycle};
		gonTime = {12'b0, gvolt} * {8'b0, toDutyCycle};
		bonTime = {12'b0, bvolt} * {8'b0, toDutyCycle};
	end

	// Use a counter to create three square waves with the given duty cycles
	// for each of the LED colors.
	always_ff @(posedge clk) begin
		if (counter <= ronTime[19:8]) pwmColors[2] <= 1;
		else pwmColors[2] <= 0;

		if (counter <= gonTime[19:8]) pwmColors[1] <= 1;
		else pwmColors[1] <= 0;

		if (counter <= bonTime[19:8]) pwmColors[0] <= 1;
		else pwmColors[0] <= 0;

		counter++;
	end
					
endmodule

module setColors(input logic [23:0] baseColor,
					input logic [7:0] sigPower,
					input logic [1:0] row, col,
					input logic isBeat, clk,
					output logic [23:0] color);

	// Hill Balliet - wballiet@g.hmc.edu - 11/20/16
    // This module outputs a digital representation of the color that should
    // be displayed at a given row and column based on the following rules.
    //
    // P type LEDs use the baseColor
    // P type LEDs are scaled by sigPower/maxSigPower to determine
	// brightness values 
	// 
    // X type LEDs use the gb values of baseColor but r = r + 20
    // X type LEDs flash when isBeat goes from 0 to 1
    // 
    // 2 type LEDs use the gb values of baseColor but r = r + 50
    // 2 type LEDs flash when is2Beat goes from 0 to 1

    logic is2Beat, wasBeat;
    logic [1:0] beatCount, multCount;
    logic [21:0] hold;
    logic [15:0] rfloat, gfloat, bfloat, product, multiplicand;

    enum {pType, xType, twoType} ledType;

    // Count beats so that every other beat is2Beat turns on
	 // Init to 0 for simulation
	 initial begin
		beatCount = 0;
		hold = 0;
		multCount = 0;
	 end
	 
	 always_ff @(posedge clk)
		if (multCount == 2'b10) multCount = 2'b00;
		else multCount++;
	 
	 always_ff @(posedge clk)
		wasBeat <= isBeat;
	 
    always_ff @(posedge clk)
		if (!wasBeat && isBeat) beatCount <= beatCount + 2'b10;
		

    assign is2Beat = beatCount[1];

    // Keep X and 2 type LEDs on for 0.1 seconds
    // Note that beats are guaranteed to be more than 0.1 seconds apart due to
    // the nature of the beat finding algorithm
    always_ff @(posedge clk)
    	if (isBeat) hold <= 22'h3D0900;
    	else if (hold > 0) hold--;

    // Determine the current LED type by looking up the current row and col in
    // the table at the top of LED type by position in the matrix.
   	always_comb
   		case ({row, col})
   			4'b0111: ledType = xType;
   			4'b1010: ledType = xType;
   			4'b1011: ledType = twoType;
   			4'b1101: ledType = xType;
   			4'b1110: ledType = twoType;
   			4'b1111: ledType = xType;
   			default: ledType = pType;
   		endcase

		floatMult rgbFloat(multiplicand, {8'b0, sigPower}, product);
			
		// Multiplex the multiplication hardware because there isn't enough space
		always_ff @(posedge clk)
			if (ledType == pType) begin
				if (multCount == 2'b00) begin
					rfloat <= product;
					multiplicand <= {8'b0, baseColor[15:8]};
				end
				else if (multCount == 2'b01) begin
					gfloat <= product;
					multiplicand <= {8'b0, baseColor[7:0]};
				end
				else begin
					bfloat <= product;
					multiplicand <= {8'b0, baseColor[23:16]};
				end
			end
			else begin
				rfloat <= 0;
				gfloat <= 0;
				bfloat <= 0;
			end
			
   	// Apply the color rules
   	always_comb
   		case (ledType)
   			pType: begin
   				// Do floating point multiplication to scale the colors
   				// maxSigPower is approx 0.00000001 in bin so has been
   				// simplified out of the calculations
   				// sigPower is 16'b00.xx and baseColor is 16'b00XX so the result
   				// will be 16'bXX.xx
   				//rfloat = {8'b0, baseColor[23:16]}  * {8'b0, sigPower};
   				//gfloat = {8'b0, baseColor[15:8]}  * {8'b0, sigPower};
   				//bfloat = {8'b0, baseColor[7:0]}  * {8'b0, sigPower};
					
   				color[23:16] = rfloat[15:8];
   				color[15:8] = gfloat[15:8];
   				color[7:0] = bfloat[15:8];
   			end
   			xType: begin
   				if (hold > 0) begin
						color = ~baseColor;
					end
					else begin
						color = 24'h0000;
					end
   			end
   			twoType: begin
					// Switch between color 2 and color 3 at every beat.
					// Color 2 is found by rotating baseColor by 1 byte
					// Color 3 is found by rotating baseColor by 2 bytes
   				if (hold > 0 && is2Beat) begin
						color[23:16] = baseColor[7:0];
						color[15:8] = baseColor[23:16];
						color[7:0] = baseColor[15:8];
					end
					else if (hold > 0) begin
						color[23:16] = baseColor[15:8];
						color[15:8] = baseColor[7:0];
						color[7:0] = baseColor[23:16];
					end
					else begin
						color = 24'h0000;
					end
   			end
   		endcase 

endmodule

module floatMult(input logic[15:0] mult1, mult2,
						output logic [15:0] product);
		assign product = mult1  * mult2;
endmodule

