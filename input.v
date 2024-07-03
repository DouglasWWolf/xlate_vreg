

/*
@register DDR control register
@field ddr_core_reset 2 0 RW 0x1 Reset the entire DDR core complex
@fdesc                           This is a continuation line
@field ddr0_reset     1 1 RW 0x1 DDR controller 0 reset
@field ddr1_reset     1 2 RW 0x1 DDR controller 1 reset
*/
localparam REG_GLOBAL_DDR_CTL = 3;



/*
@register Test of full register
@field dat           32 0 RW 0   Full register data
*/
localparam REG_GEN_DATA = 4;

