module pllClk(CLOCK_50, KEY, GPIO_0);
	input CLOCK_50;
	input [0:0] KEY; //use KEY0 as resetn
	output [27:0]GPIO_0;
	wire locked;
	
	wire CLOCK;
	wire resetn;
	
	assign resetn = KEY[0];
	pllIP myPLL (
		CLOCK_50,   //  refclk.clk
		resetn,      //   reset.reset
		CLOCK, // outclk0.clk
		locked    //  locked.export
	);
	assign GPIO_0[26] = CLOCK;
endmodule
