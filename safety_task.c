#include "gate_control.h"

void safetyTask(void *pvParameters) {
    for (;;) {
        // Wait for the obstacle button to be pressed
        if (xSemaphoreTake(xObstacleSemaphore, portMAX_DELAY) == pdPASS) {
            
            // Take the state mutex to safely check and alter the state
            xSemaphoreTake(xGateStateMutex, portMAX_DELAY);
            
            // Obstacle logic only triggers if the gate is actively closing
            if (currentGateState == CLOSING) { 
                currentGateState = REVERSING;  
							
								// Elevate LED Task priority to 5 (higher than safety task)
                vTaskPrioritySet(xLedTaskHandle, 5);
                
                // CRITICAL: Release the mutex before the delay so the LED task can run!
                xSemaphoreGive(xGateStateMutex);
                xSemaphoreGive(xLedSemaphore);

                // Reverse for 0.5 seconds
                vTaskDelay(pdMS_TO_TICKS(500)); 
                
                // Take mutex again to perform the final stop
                xSemaphoreTake(xGateStateMutex, portMAX_DELAY);
                currentGateState = STOPPED_MIDWAY;
                xSemaphoreGive(xGateStateMutex);
                xSemaphoreGive(xLedSemaphore);
							
								// Restore LED Task priority to original (2)
                vTaskPrioritySet(xLedTaskHandle, 2);
							
            } else {
                // If the gate is OPENING or stopped, ignore the obstacle and release the mutex
                xSemaphoreGive(xGateStateMutex); 
            }
        }
    }
}

