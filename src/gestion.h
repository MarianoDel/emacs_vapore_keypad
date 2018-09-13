//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### GESTION.H #################################
//------------------------------------------------------
#ifndef _GESTION_H_
#define _GESTION_H_

// Additional Includes for Configuration ------------------------------


// Defines for Configuration ----------------------------------------



// Exported Macros and Defines ----------------------------------------
//estados de la SM de GESTION
typedef enum {
    GESTION_SM_INIT = 0,
    GESTION_SM_TO_MONITORING,
    GESTION_SM_TO_MONITORINGA,		
    GESTION_SM_TO_MONITORINGB,
    GESTION_SM_TO_MONITORINGB2,
    GESTION_SM_TO_MONITORINGC,

    GESTION_SM_MAIN,
    GESTION_SM_TO_MAIN_TIMEOUT,

    GESTION_SM_TO_FLUSH_SST,
    GESTION_SM_TO_WRITE_SST_CONF,
    GESTION_SM_TO_WRITE_SST_CONFA,
    GESTION_SM_TO_WRITE_SST_CONFB,
    GESTION_SM_TO_WRITE_SST_CONFC,

    GESTION_SM_TO_WRITE_SST0,
    GESTION_SM_TO_WRITE_SST0_A,	
    GESTION_SM_TO_WRITE_SST0_B,
    GESTION_SM_TO_WRITE_SST0_C,

    GESTION_SM_TO_WRITE_SST1,
    GESTION_SM_TO_WRITE_SST1_A,	
    GESTION_SM_TO_WRITE_SST1_B,

    GESTION_SM_TO_WRITE_SST2,
    GESTION_SM_TO_WRITE_SST2_A,	
    GESTION_SM_TO_WRITE_SST2_B,	

    GESTION_SM_TO_WRITE_SST3,		
    GESTION_SM_TO_WRITE_SST3_A,	
    GESTION_SM_TO_WRITE_SST3_B,	

    GESTION_SM_TO_WRITE_SST4,		
    GESTION_SM_TO_WRITE_SST4_A,	
    GESTION_SM_TO_WRITE_SST4_B,	

    GESTION_SM_TO_WRITE_SST5,		
    GESTION_SM_TO_WRITE_SST5_A,	
    GESTION_SM_TO_WRITE_SST5_B,	

    GESTION_SM_TO_WRITE_SST6,		
    GESTION_SM_TO_WRITE_SST6_A,	
    GESTION_SM_TO_WRITE_SST6_B,	

    GESTION_SM_TO_WRITE_SST7,		
    GESTION_SM_TO_WRITE_SST7_A,	
    GESTION_SM_TO_WRITE_SST7_B,

    GESTION_SM_TO_WRITE_SST8,
    GESTION_SM_TO_WRITE_SST8_A,	
    GESTION_SM_TO_WRITE_SST8_B,	

    GESTION_SM_TO_WRITE_SST9,		
    GESTION_SM_TO_WRITE_SST9_A,	
    GESTION_SM_TO_WRITE_SST9_B,	
    
    GESTION_SM_TO_MONITORING_LEAVE,
    GESTION_SM_TO_MONITORINGE,
    
    GESTION_SM_TO_SAVE_TIME,

    GESTION_SM_TO_MAIN_WAIT_5SEGS,
    GESTION_SM_TO_MAIN_WAIT_5SEGSA,
    
    GESTION_SM_TO_MAIN_OK

} gestion_sm_t;







// Exported Module Functions ------------------------------------------
void FuncGestion (void);


#endif    /* _GESTION_H_ */

//--- end of file ---//
