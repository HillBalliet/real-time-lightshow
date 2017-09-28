module finalProject (input logic clk, sdi, sclk, notLoad, isBeat,
								output logic [11:0] rgbSig,
								//output logic [2:0] testLEDs,
								output logic [3:0] enables);
    // Hill Balliet - wballiet@g.hmc.edu - 11/20/16
    // This is the top level module and it takes in microphone data and color
    // data from a SPI connection and then powers an LED matrix as a
    // visualization of the microphone data.
	 //
	 // the bits of rgbSig correspond to {r18, g18, b18, r27, g27, b27, r36, g36, b36, r45, g45, b45} from MSB to LSB
	 // the bits of enables correspond to {EN18, EN27, EN36, EN45} from MSB to LSB
	 logic [7:0] red, green, blue, power;
	 logic [11:0] notRGBSig;
	 logic load;
	 
	 assign load = ~notLoad;
	 
    spiColPowIn spiColPow(sclk, sdi, load, clk, red, green, blue, power);
	 // For debugging with the test LEDs
    //arrayDriver leds(red, green, blue, power, clk, isBeat, notRGBSig, testLEDs, enables);
	 arrayDriver leds(red, green, blue, power, clk, isBeat, notRGBSig, enables);
	 
	 // arrayDriver works for common anode LED arrays, but we are using a common cathode LED array
	 assign rgbSig = ~notRGBSig;
endmodule

module testbench();

    logic sampclk, clk, sdi, sclk, load, newData, lastSampClk, nextSampClk;
    logic [4:0] sclkCount, shifts;
    logic [7:0] enables, micData;
	 logic [9:0] sampNum;
    logic [11:0] rgbSig;
    logic [23:0] baseColor;

    // test micData implemented as a ROM
    logic [7:0] data[0:1025];

    finalProject dut(clk, sdi, sclk, load, newData, micData, rgbSig, enables);

    // generate clock and sampclk signals
    initial begin
        forever begin
            clk = 1'b0; #5;
            clk = 1'b1; #5;
        end
	end
	initial begin
        forever begin
            sampclk = 1'b0; #5000;
            sampclk = 2'b1; #5000;
        end
	end
	initial begin
        baseColor = 24'h4286f4;
        load = 1'b1;
        shifts = 0;
        sampNum = 0;
        sclkCount = 0;
        $readmemh("data.txt", data);
    end

    // Create sclk with a clk divider
    always_ff @(posedge clk)
        if (load) sclkCount++;
        else sclkCount <= 0;

    assign sclk = sclkCount[4];
	 
	 // Create the newData signal
	 always_ff @(posedge clk) begin
			lastSampClk <= nextSampClk;
			nextSampClk <= sampclk;
	 end
	 
	 always_comb
		if (nextSampClk == 1 && lastSampClk == 0) newData = 1;
		else newData = 0;

    // Keep track of how many times we've shifted and turn load off after 24
    always_ff @(posedge sclk)
        shifts++;

    always_ff @(posedge clk)
        if (shifts == 25) load <= 0;

    // Put the correct sample into micData
    always_ff @(posedge sampclk)
        if (sampNum < 1025) sampNum++;
        else begin
            $display("Testbench ran to completion");
            $stop();
        end

    assign micData = data[sampNum];

    // Send over the color
    always_ff @(negedge sclk)
        sdi <= baseColor[23-shifts+1];

endmodule


module spiColPowIn(input logic sck, sdi, load, clk,
                    output logic [7:0] red, green, blue, power);
    logic [7:0] presyncR, presyncG, presyncB, presyncP, syncR, syncG, syncB, syncP;
	 
	 
    always_ff @(posedge sck)
        if (load) begin
			presyncR[7:1] <= presyncR[6:0];
			presyncR[0] <= presyncG[7];
			presyncG[7:1] <= presyncG[6:0];
			presyncG [0] <= presyncB[7];
			presyncB[7:1] <= presyncB[6:0];
			presyncB[0] <= presyncP[7];
			presyncP[7:1] <= presyncP[6:0];
			presyncP[0] <= sdi;
		  end
	
	// Synchronize the input from SPI
	always_ff @(posedge clk) begin
		  red <= syncR;
		  green <= syncG;
		  blue <= syncB;
		  power <= syncP;
		  syncR <= presyncR;
		  syncG <= presyncG;
		  syncB <= presyncB;
		  syncP <= presyncP;
	end
    
endmodule
