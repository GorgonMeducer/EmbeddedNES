/*----------------------------------------------------------------------------
 * Name:    Secure_Functions.h
 * Purpose: Function and Typedef Declarations to include into NonSecure Application.
 *----------------------------------------------------------------------------*/

/* Define typedef for NonSecure callback function */ 
typedef int32_t (*NonSecure_funcptr)(uint32_t);

/* Function declarations for Secure functions called from NonSecure application */
extern int32_t Secure_LED_On (uint32_t);
extern int32_t Secure_LED_Off(uint32_t);
extern int32_t Secure_LED_On_callback (NonSecure_funcptr);
extern int32_t Secure_LED_Off_callback(NonSecure_funcptr);
