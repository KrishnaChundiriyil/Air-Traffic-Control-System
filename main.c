#include "raylib.h"
#include "raymath.h" // Include for Lerp function
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h> // Include for tolower

// --- Existing Macros ---
#define MAX_LINE_SIZE 2048
#define MAX_RECORDS 1000
#define MAX_STR_LEN 256
#define DOMESTIC_TEAMS 12
#define INTERNATIONAL_TEAMS 8
#define BACKUP_TEAMS 10
#define DOMESTIC_SIZE 5
#define INTERNATIONAL_SIZE 8
#define REST_HOURS 8
#define INITIAL_RESTING_CREWS 8
#define NUM_SLOTS 5
#define MAX_CAPACITY_PER_SLOT 9
#define NUM_RUNWAYS 3
#define MAX_PER_RUNWAY 4
#define NORMAL_BUFFER_TIME 20
#define REDUCED_BUFFER_TIME 15
#define SLOT_DURATION 60
#define MAX_OUTPUT_SIZE 16384
#define MAX_INPUT_LENGTH 50

// --- UI Enhancement Defines ---
#define BASE_FONT_SIZE 20
#define PADDING 10
#define SCROLLBAR_WIDTH 12
#define BUTTON_HEIGHT 40
#define ROUNDNESS 0.1f
#define SEGMENTS 10 // For rounded rectangles

// --- Professional Color Palette ---
#define COLOR_BACKGROUND      CLITERAL(Color){ 240, 242, 245, 255 } // Light Grayish Blue
#define COLOR_PANEL_BG        CLITERAL(Color){ 255, 255, 255, 255 } // White
#define COLOR_MENU_BG         CLITERAL(Color){ 230, 233, 237, 255 } // Slightly darker panel
#define COLOR_ACCENT          CLITERAL(Color){ 66, 133, 244, 255 }  // Google Blue
#define COLOR_ACCENT_HOVER    CLITERAL(Color){ 100, 157, 245, 255 } // Lighter Blue
#define COLOR_BUTTON_NORMAL   CLITERAL(Color){ 225, 227, 230, 255 } // Light Gray
#define COLOR_BUTTON_HOVER    CLITERAL(Color){ 205, 209, 214, 255 } // Darker Gray
#define COLOR_TEXT_DARK       CLITERAL(Color){ 32, 33, 36, 255 }    // Almost Black
#define COLOR_TEXT_LIGHT      CLITERAL(Color){ 250, 250, 250, 255 } // Off White
#define COLOR_TEXT_MEDIUM     CLITERAL(Color){ 95, 99, 104, 255 }   // Medium Gray
#define COLOR_BORDER          CLITERAL(Color){ 218, 220, 224, 255 } // Light Gray Border
#define COLOR_SCROLLBAR_BG    CLITERAL(Color){ 235, 235, 235, 255 }
#define COLOR_SCROLLBAR_THUMB CLITERAL(Color){ 180, 180, 180, 255 }
#define COLOR_SCROLLBAR_HOVER CLITERAL(Color){ 150, 150, 150, 255 }
#define COLOR_EMERGENCY_BG    CLITERAL(Color){ 255, 235, 238, 255 } // Light Pink
#define COLOR_EMERGENCY_TEXT  CLITERAL(Color){ 198, 40, 40, 255 }   // Dark Red
#define COLOR_HEADER_BG       CLITERAL(Color){ 248, 249, 250, 255 } // Very Light Gray
#define COLOR_INPUT_BG        WHITE
#define COLOR_INPUT_ACTIVE_BORDER COLOR_ACCENT
#define COLOR_PROMPT_OVERLAY  CLITERAL(Color){ 0, 0, 0, 120 }      // Semi-transparent black
#define COLOR_YES             CLITERAL(Color){ 52, 168, 83, 255 }   // Google Green
#define COLOR_NO              CLITERAL(Color){ 234, 67, 53, 255 }   // Google Red

// --- Structures (Unchanged) ---
typedef struct {
    char airline[MAX_STR_LEN];
    char plane_domain[MAX_STR_LEN];
    char plane_id[MAX_STR_LEN];
    int arr_or_dip;
    char starting_destination[MAX_STR_LEN];
    char end_destination[MAX_STR_LEN];
    char central_destination[MAX_STR_LEN];
    int slot_order; // Can be requested slot initially, then assigned slot (0=unalloc, -1=rerouted, -2=unaccom)
    int emergency;
    int runway_assignment; // 0=unassigned, 1-based index, or <0 for special status matching slot_order?
    int delay_minutes;
} AirlineRecord;

typedef struct {
    int team_id;
    int members;
    int available;
    int rest_time;
} CrewTeam;

typedef struct {
    int slot_number;
    int planes_allocated;
    int international_count;
    int domestic_count;
    int max_capacity;
    int runway_assignments[NUM_RUNWAYS]; // Count per runway in this slot
} SlotStatus;

typedef enum { AUTH_SELECT, AUTH_LOGIN, MAIN_MENU } AppState;
typedef enum { ROLE_NONE, ROLE_ADMIN, ROLE_USER } UserRole;

typedef struct {
    AirlineRecord records[MAX_RECORDS];
    CrewTeam domestic[DOMESTIC_TEAMS];
    CrewTeam international[INTERNATIONAL_TEAMS];
    CrewTeam backup[BACKUP_TEAMS];
    SlotStatus slots[NUM_SLOTS];
    int recordCount;
    int initialized;
    int dataLoaded;
    int slotsAllocated;
} AppData;

// --- Function Prototypes ---
void initializeCrewTeams(CrewTeam domestic[], CrewTeam international[], CrewTeam backup[]);
void updateCrewStatus(CrewTeam domestic[], CrewTeam international[], CrewTeam backup[]);
int allocateCrew(int isInternational, CrewTeam domestic[], CrewTeam international[], CrewTeam backup[]);
void printCrewStatus(CrewTeam domestic[], CrewTeam international[], CrewTeam backup[], char output[], int max_len);
int compareBySlot(const void *a, const void *b);
int readCSV(const char* filename, AirlineRecord records[], int maxRecords);
void processFlightsBySlot(AirlineRecord records[], int count, CrewTeam domestic[], CrewTeam international[], CrewTeam backup[], char output[], int max_len);
void displayFlightDetails(AirlineRecord records[], int count, char output[], int* line_count, float* scroll_offset, int x, int y, int width, int height);
void initializeSlots(SlotStatus slots[], int num_slots, int max_capacity);
void printSlotStatus(SlotStatus slots[], int num_slots, char output[], int* line_count, float* scroll_offset, int x, int y, int width, int height);
int compareFlightPriority(const void *a, const void *b);
void slotAllocator(AirlineRecord records[], int count, SlotStatus slots[], int num_slots);
void processFlightsWithSlotAllocation(AirlineRecord records[], int count, CrewTeam domestic[], CrewTeam international[], CrewTeam backup[], char output[], int max_len);
void runwayPlanner(AirlineRecord records[], int count, SlotStatus slots[], int num_slots);
void displayRunwayAllocationPlan(AirlineRecord records[], int count, SlotStatus slots[], int num_slots, char output[], int max_len, int x, int y, int width, int height, float* scroll_offset);
void airplaneDelay(AirlineRecord records[], int count, SlotStatus slots[], int num_slots, char output[], int max_len);
void simulateSlotWithDelays(AirlineRecord records[], int count, SlotStatus slots[], int num_slots, char output[], int max_len);
void saveFlightAndSlotDetailsToFile(AirlineRecord records[], int count, SlotStatus slots[], int num_slots, char output[], int max_len);

// --- UI Helper Functions ---
void drawTableCell(const char* text, int x, int y, int width, int height, int font_size, Color text_color, bool is_header, bool is_hovered, bool is_emergency);
void appendToOutput(char output[], int max_len, const char *new_text, int *output_lines);
void drawWrappedText(const char* text, int x, int y, int max_width, int font_size, Color color, float scroll_offset, int max_height);
int calculateWrappedTextHeight(const char* text, int max_width, int font_size);
void DrawButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor, Color textColor, bool hovered, bool selected, Color selectedColor);
void DrawTextInput(Rectangle bounds, const char *label, char *text, int maxLength, bool active);
void DrawPanel(Rectangle bounds, Color color);
void DrawScrollbar(Rectangle bounds, float scrollOffset, float contentHeight, float viewHeight, float *targetScrollOffset, bool *isDragging);

// --- Core Logic Functions (Implementations - Unchanged from previous correct versions) ---
// ... (Paste ALL function implementations from the previous response here) ...
// ... (Including: appendToOutput, initializeCrewTeams, updateCrewStatus, allocateCrew, ...)
// ... (printCrewStatus, compareBySlot, readCSV, processFlightsBySlot, displayFlightDetails, ...)
// ... (initializeSlots, printSlotStatus, compareFlightPriority, slotAllocator, ...)
// ... (processFlightsWithSlotAllocation, runwayPlanner, displayRunwayAllocationPlan, ...)
// ... (airplaneDelay, simulateSlotWithDelays, saveFlightAndSlotDetailsToFile)
// --- UI Helper Functions (Implementations - Unchanged from previous correct versions) ---
// ... (Paste drawTableCell, drawWrappedText, calculateWrappedTextHeight, DrawButton, ...)
// ... (DrawTextInput, DrawPanel, DrawScrollbar implementations here)

// Helper function to append to the output buffer
void appendToOutput(char output[], int max_len, const char *new_text, int *output_lines) {
    if (!new_text) return; // Safety check
    int current_len = strlen(output);
    int new_text_len = strlen(new_text);

    if (current_len + new_text_len + 1 < max_len) {
        strcat(output, new_text);
        if (output_lines) {
            for (int i = 0; i < new_text_len; i++) {
                if (new_text[i] == '\n') (*output_lines)++;
            }
        }
    } else if (max_len > current_len) { // Check if any space remaining
        // Try to append at least the truncation message
        const char *trunc_msg = "\n[Output truncated]\n";
        int trunc_len = strlen(trunc_msg);
        if (max_len > current_len + trunc_len + 1) {
             strcat(output, trunc_msg);
              if (output_lines) (*output_lines) += 2;
        } else {
            // Not even space for truncation message, just ensure termination
            output[max_len - 1] = '\0';
        }
    }
     // Ensure null termination is always within bounds
     output[max_len-1] = '\0';
}

// Initialize Crew Teams
void initializeCrewTeams(CrewTeam domestic[], CrewTeam international[], CrewTeam backup[]) {
    int resting_count = 0;
    srand(time(NULL));

    for (int i = 0; i < DOMESTIC_TEAMS; i++) {
        domestic[i].team_id = i + 1;
        domestic[i].members = DOMESTIC_SIZE;
        if (resting_count < INITIAL_RESTING_CREWS && rand() % 3 == 0) {
            domestic[i].available = 0;
            domestic[i].rest_time = rand() % REST_HOURS + 1;
            resting_count++;
        } else {
            domestic[i].available = 1;
            domestic[i].rest_time = 0;
        }
    }

    for (int i = 0; i < INTERNATIONAL_TEAMS; i++) {
        international[i].team_id = i + 1;
        international[i].members = INTERNATIONAL_SIZE;
        if (resting_count < INITIAL_RESTING_CREWS && rand() % 3 == 0) {
            international[i].available = 0;
            international[i].rest_time = rand() % REST_HOURS + 1;
            resting_count++;
        } else {
            international[i].available = 1;
            international[i].rest_time = 0;
        }
    }

    for (int i = 0; i < BACKUP_TEAMS; i++) {
        backup[i].team_id = i + 1;
        backup[i].members = (i % 2 == 0) ? DOMESTIC_SIZE : INTERNATIONAL_SIZE;
        if (resting_count < INITIAL_RESTING_CREWS && rand() % 3 == 0) {
            backup[i].available = 0;
            backup[i].rest_time = rand() % REST_HOURS + 1;
            resting_count++;
        } else {
            backup[i].available = 1;
            backup[i].rest_time = 0;
        }
    }

    // Ensure exactly INITIAL_RESTING_CREWS are resting initially
    while (resting_count < INITIAL_RESTING_CREWS) {
        int team_type = rand() % 3; // 0: domestic, 1: international, 2: backup
        bool assigned_rest = false;

        if (team_type == 0) {
            for (int i = 0; i < DOMESTIC_TEAMS && !assigned_rest; i++) {
                if (domestic[i].available) {
                    domestic[i].available = 0;
                    domestic[i].rest_time = rand() % REST_HOURS + 1;
                    resting_count++;
                    assigned_rest = true;
                }
            }
        } else if (team_type == 1) {
            for (int i = 0; i < INTERNATIONAL_TEAMS && !assigned_rest; i++) {
                if (international[i].available) {
                    international[i].available = 0;
                    international[i].rest_time = rand() % REST_HOURS + 1;
                    resting_count++;
                    assigned_rest = true;
                }
            }
        } else { // Backup teams
            for (int i = 0; i < BACKUP_TEAMS && !assigned_rest; i++) {
                if (backup[i].available) {
                    backup[i].available = 0;
                    backup[i].rest_time = rand() % REST_HOURS + 1;
                    resting_count++;
                    assigned_rest = true;
                }
            }
        }

         // Failsafe: if no available team found in the chosen type, try others next iteration
         // Or break if literally no teams are available anymore (shouldn't happen here)
         bool any_available = false;
         for(int i=0; i<DOMESTIC_TEAMS; i++) if(domestic[i].available) any_available=true;
         for(int i=0; i<INTERNATIONAL_TEAMS; i++) if(international[i].available) any_available=true;
         for(int i=0; i<BACKUP_TEAMS; i++) if(backup[i].available) any_available=true;
         if (!any_available && resting_count < INITIAL_RESTING_CREWS) {
             fprintf(stderr, "Warning: Could not assign initial resting crews - not enough available teams.\n");
             break; // Avoid infinite loop
         }
    }
}


// Update Crew Status
void updateCrewStatus(CrewTeam domestic[], CrewTeam international[], CrewTeam backup[]) {
    for (int i = 0; i < DOMESTIC_TEAMS; i++) {
        if (!domestic[i].available && domestic[i].rest_time > 0) {
            domestic[i].rest_time--;
            if (domestic[i].rest_time == 0) {
                domestic[i].available = 1;
            }
        }
    }

    for (int i = 0; i < INTERNATIONAL_TEAMS; i++) {
        if (!international[i].available && international[i].rest_time > 0) {
            international[i].rest_time--;
            if (international[i].rest_time == 0) {
                international[i].available = 1;
            }
        }
    }

    for (int i = 0; i < BACKUP_TEAMS; i++) {
        if (!backup[i].available && backup[i].rest_time > 0) {
            backup[i].rest_time--;
            if (backup[i].rest_time == 0) {
                backup[i].available = 1;
            }
        }
    }
}

// Allocate Crew
int allocateCrew(int isInternational, CrewTeam domestic[], CrewTeam international[], CrewTeam backup[]) {
    CrewTeam *teams = isInternational ? international : domestic;
    int totalTeams = isInternational ? INTERNATIONAL_TEAMS : DOMESTIC_TEAMS;
    int requiredSize = isInternational ? INTERNATIONAL_SIZE : DOMESTIC_SIZE;

    // Prioritize main teams
    for (int i = 0; i < totalTeams; i++) {
        if (teams[i].available) {
            teams[i].available = 0;
            teams[i].rest_time = REST_HOURS;
            return teams[i].team_id; // Return assigned team ID
        }
    }

    // Use backup teams if main teams unavailable
    for (int i = 0; i < BACKUP_TEAMS; i++) {
        if (backup[i].available && backup[i].members == requiredSize) {
            backup[i].available = 0;
            backup[i].rest_time = REST_HOURS;
            return -(backup[i].team_id); // Return negative ID for backup teams
        }
    }

    return 0; // No crew available
}

// Print Crew Status
void printCrewStatus(CrewTeam domestic[], CrewTeam international[], CrewTeam backup[], char output[], int max_len) {
    char temp_buffer[4096] = {0};
    int pos = 0;
    int total_domestic_available = 0, total_international_available = 0, total_backup_available = 0;
    int total_domestic_resting = 0, total_international_resting = 0, total_backup_resting = 0;

    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "--- Current Crew Status ---\n\n");

    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "** Domestic Teams (%d Total) **\n", DOMESTIC_TEAMS);
    for (int i = 0; i < DOMESTIC_TEAMS; i++) {
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Team D%d: %s", domestic[i].team_id, domestic[i].available ? "[Available]" : "[Resting]");
        if (!domestic[i].available) {
            pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, " (%dh left)", domestic[i].rest_time);
            total_domestic_resting++;
        } else {
            total_domestic_available++;
        }
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n");
        if (pos >= sizeof(temp_buffer) - 100) goto end_print; // Prevent buffer overflow
    }
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Summary: %d Available, %d Resting\n\n", total_domestic_available, total_domestic_resting);
     if (pos >= sizeof(temp_buffer) - 100) goto end_print;

    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "** International Teams (%d Total) **\n", INTERNATIONAL_TEAMS);
    for (int i = 0; i < INTERNATIONAL_TEAMS; i++) {
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Team I%d: %s", international[i].team_id, international[i].available ? "[Available]" : "[Resting]");
        if (!international[i].available) {
            pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, " (%dh left)", international[i].rest_time);
            total_international_resting++;
        } else {
            total_international_available++;
        }
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n");
        if (pos >= sizeof(temp_buffer) - 100) goto end_print;
    }
     pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Summary: %d Available, %d Resting\n\n", total_international_available, total_international_resting);
      if (pos >= sizeof(temp_buffer) - 100) goto end_print;


    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "** Backup Teams (%d Total) **\n", BACKUP_TEAMS);
    for (int i = 0; i < BACKUP_TEAMS; i++) {
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Team B%d: %s", backup[i].team_id, backup[i].available ? "[Available]" : "[Resting]");
        if (!backup[i].available) {
            pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, " (%dh left)", backup[i].rest_time);
             total_backup_resting++;
        } else {
             total_backup_available++;
        }
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, " (%s size)\n", backup[i].members == DOMESTIC_SIZE ? "Domestic" : "International");
        if (pos >= sizeof(temp_buffer) - 100) goto end_print;
    }
     pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Summary: %d Available, %d Resting\n", total_backup_available, total_backup_resting);

end_print:
    temp_buffer[pos] = '\0'; // Null-terminate safely
    appendToOutput(output, max_len, temp_buffer, NULL); // Use append function
}


// Compare By Slot (Prioritize Emergency, then Slot Order)
int compareBySlot(const void *a, const void *b) {
    AirlineRecord *recordA = (AirlineRecord *)a;
    AirlineRecord *recordB = (AirlineRecord *)b;

    // Handle special slot values first (rerouted, unallocated etc should maybe go last)
    // Treat 0, -1, -2 etc. as lower priority than positive slots
    bool a_is_positive = recordA->slot_order > 0;
    bool b_is_positive = recordB->slot_order > 0;

    if (a_is_positive && !b_is_positive) return -1; // A is normal, B is special -> A goes before B
    if (!a_is_positive && b_is_positive) return 1;  // A is special, B is normal -> A goes after B
    if (!a_is_positive && !b_is_positive) {
        // Both special: Sort 0 (unallocated) after -1 (rerouted) after -2 (unaccom)
        return recordA->slot_order - recordB->slot_order; // Smaller negative number comes first
    }

    // Both have valid positive slots
    // Then by slot order
    if (recordA->slot_order != recordB->slot_order) {
        return recordA->slot_order - recordB->slot_order;
    }

    // Within the same slot, Emergency flights first
    if (recordA->emergency != recordB->emergency) {
        return recordB->emergency - recordA->emergency; // Higher emergency value comes first
    }

     // Optional: If slots and emergency are same, maybe sort by priority?
     int aIntl = (strcmp(recordA->plane_domain, "international") == 0);
     int bIntl = (strcmp(recordB->plane_domain, "international") == 0);
     if (aIntl != bIntl) {
         return bIntl - aIntl; // International (1) comes before Domestic (0)
     }

    return 0; // Keep original relative order if all else is equal
}

// Read CSV
int readCSV(const char* filename, AirlineRecord records[], int maxRecords) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening CSV file"); // More informative error
        return -1;
    }

    char line[MAX_LINE_SIZE];
    int recordCount = 0;
    int line_number = 0;

    // Read header line (and potentially skip it)
    if (!fgets(line, MAX_LINE_SIZE, file)) {
        fclose(file);
        return 0; // Empty file
    }
    line_number++;

    // Basic header check (can be improved)
    // Convert line to lower case for case-insensitive check
    char line_lower[MAX_LINE_SIZE];
    strncpy(line_lower, line, MAX_LINE_SIZE - 1);
    line_lower[MAX_LINE_SIZE - 1] = '\0';
    for(int i = 0; line_lower[i]; i++){ line_lower[i] = tolower(line_lower[i]); }

    if (strstr(line_lower, "airline") == NULL && strstr(line_lower, "plane") == NULL) {
        rewind(file); // Assume no header if common fields aren't found
        line_number = 0; // Reset line count if rewinding
    }

    while (fgets(line, MAX_LINE_SIZE, file) && recordCount < maxRecords) {
        line_number++;
        // Clean the record structure before parsing
        memset(&records[recordCount], 0, sizeof(AirlineRecord));
        records[recordCount].runway_assignment = 0; // Explicitly set default
        records[recordCount].delay_minutes = 0;

        // Handle potential trailing newline/carriage return
        line[strcspn(line, "\r\n")] = 0;

        // Skip empty lines
        if (strlen(line) == 0) continue;

        char *token;
        char *rest = line;
        int field = 0;

        token = strtok(rest, ","); // Use rest directly with strtok first time
        while (token != NULL && field < 9) { // Limit fields to expected number
             // Trim leading/trailing whitespace
            while (*token == ' ' || *token == '\t') token++;
            char *end = token + strlen(token) - 1;
            while (end >= token && (*end == ' ' || *end == '\t')) *end-- = '\0';

            // Assign to fields, checking for empty tokens after trimming
            if (strlen(token) > 0) {
                switch (field) {
                    case 0: strncpy(records[recordCount].airline, token, MAX_STR_LEN - 1); records[recordCount].airline[MAX_STR_LEN - 1] = '\0'; break;
                    case 1: strncpy(records[recordCount].plane_domain, token, MAX_STR_LEN - 1); records[recordCount].plane_domain[MAX_STR_LEN - 1] = '\0'; break;
                    case 2: strncpy(records[recordCount].plane_id, token, MAX_STR_LEN - 1); records[recordCount].plane_id[MAX_STR_LEN - 1] = '\0'; break;
                    case 3: records[recordCount].arr_or_dip = atoi(token); break;
                    case 4: strncpy(records[recordCount].starting_destination, token, MAX_STR_LEN - 1); records[recordCount].starting_destination[MAX_STR_LEN - 1] = '\0'; break;
                    case 5: strncpy(records[recordCount].end_destination, token, MAX_STR_LEN - 1); records[recordCount].end_destination[MAX_STR_LEN - 1] = '\0'; break;
                    case 6: strncpy(records[recordCount].central_destination, token, MAX_STR_LEN - 1); records[recordCount].central_destination[MAX_STR_LEN - 1] = '\0'; break;
                    case 7: records[recordCount].slot_order = atoi(token); break; // This is REQUESTED slot from file
                    case 8: records[recordCount].emergency = atoi(token); break;
                }
            }
            field++;
            token = strtok(NULL, ","); // Get next token
        }
         // Basic validation after parsing line
         if (strlen(records[recordCount].plane_id) == 0) {
             fprintf(stderr, "Warning: Skipping record line %d due to missing plane ID\n", line_number);
             memset(&records[recordCount], 0, sizeof(AirlineRecord)); // Clear partially filled record
             continue; // Skip this record
         }
         // Allow slot_order 0 for emergencies, but skip non-emergency with slot <= 0
         if (records[recordCount].slot_order <= 0 && !records[recordCount].emergency) {
             fprintf(stderr, "Warning: Skipping record line %d (%s) due to invalid requested slot (%d) for non-emergency.\n", line_number, records[recordCount].plane_id, records[recordCount].slot_order);
              memset(&records[recordCount], 0, sizeof(AirlineRecord));
             continue; // Skip if non-emergency has invalid requested slot
         }


        recordCount++;
    }

    fclose(file);
    return recordCount;
}

// Process Flights By Slot (Original Logic - using requested slot)
void processFlightsBySlot(AirlineRecord records[], int count, CrewTeam domestic[], CrewTeam international[], CrewTeam backup[], char output[], int max_len) {
    // This function is less relevant now with the proper allocation logic,
    // but kept for potential historical comparison or if specifically needed.
    // Ensure it uses a consistent understanding of slot_order if called.
    char temp_buffer[4096] = {0};
    int pos = 0;
    int successfulFlights = 0;
    int cancelledFlights = 0;
    int departureCount = 0;
    int arrivalCount = 0;

    // Create a temporary copy to sort by requested slot without modifying main array's assigned slots
    AirlineRecord *temp_records = malloc(count * sizeof(AirlineRecord));
     if (!temp_records) {
         appendToOutput(output, max_len, "Error: Memory allocation failed in processFlightsBySlot.\n", NULL);
         return;
     }
     // Need to store the original requested slot IF slot_order in records[] might be overwritten
     // Assuming records[i].slot_order holds the *requested* slot when this is called.
     memcpy(temp_records, records, count * sizeof(AirlineRecord));


    for (int i = 0; i < count; i++) {
        if (temp_records[i].arr_or_dip == 0) { // 0 = departure
            departureCount++;
        } else {
            arrivalCount++;
        }
    }

    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Read %d records: %d departures, %d arrivals.\nProcessing based on *requested* slot order (Legacy Method).\n",
                    count, departureCount, arrivalCount);

    // Sort temporary copy by requested slot
    qsort(temp_records, count, sizeof(AirlineRecord), compareBySlot);

    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n--- Processing Flights by Requested Slot (Legacy) ---\n");

    int max_slot_requested = 0;
    for(int i=0; i<count; ++i) if(temp_records[i].slot_order > max_slot_requested) max_slot_requested = temp_records[i].slot_order;
    int max_slot_to_process = (max_slot_requested > NUM_SLOTS) ? NUM_SLOTS : max_slot_requested;
     if (max_slot_to_process <= 0 && count > 0) max_slot_to_process = NUM_SLOTS;


    for (int currentSlot = 1; currentSlot <= max_slot_to_process; currentSlot++) {
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n=== Processing Slot %d ===\n", currentSlot);
        if (pos >= sizeof(temp_buffer) - 200) goto end_process_slot_legacy;

        updateCrewStatus(domestic, international, backup); // Update crew status

        bool processed_in_slot = false;
        for (int i = 0; i < count; i++) {
             // Process flight if its *requested* slot matches current slot
            if (temp_records[i].slot_order == currentSlot) {
                processed_in_slot = true;
                bool isInternational = (strcmp(temp_records[i].plane_domain, "international") == 0);

                if (temp_records[i].arr_or_dip == 0) { // Departure
                    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n[%s] DEP: %s (%s) | %s -> %s",
                                    temp_records[i].emergency ? "EMERG" : "Norml",
                                    temp_records[i].plane_id, temp_records[i].airline,
                                    temp_records[i].starting_destination, temp_records[i].end_destination);
                    if (pos >= sizeof(temp_buffer) - 200) goto end_process_slot_legacy;

                    int crewResult = allocateCrew(isInternational, domestic, international, backup);
                    if (crewResult != 0) {
                        successfulFlights++;
                        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, " -> Crew OK (Team %c%d)\n",
                                        crewResult < 0 ? 'B' : (isInternational ? 'I' : 'D'), abs(crewResult));
                    } else {
                        cancelledFlights++;
                        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, " -> Crew FAILED -> CANCELLED\n");
                    }
                     if (pos >= sizeof(temp_buffer) - 200) goto end_process_slot_legacy;

                } else { // Arrival
                    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n[%s] ARR: %s (%s) | %s -> %s via %s -> OK (No Crew Needed)\n",
                                    temp_records[i].emergency ? "EMERG" : "Norml",
                                    temp_records[i].plane_id, temp_records[i].airline,
                                    temp_records[i].starting_destination, temp_records[i].end_destination, temp_records[i].central_destination);
                     if (pos >= sizeof(temp_buffer) - 200) goto end_process_slot_legacy;
                }
            }
        }
         if (!processed_in_slot) {
             pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "(No flights requested for this slot)\n");
         }
    }

    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n--- Flight Operations Summary (Requested Slots - Legacy) ---\n");
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Total flights processed: %d\n", count);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Total departures needing crew: %d\n", departureCount);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Successful departures (crew allocated): %d\n", successfulFlights);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Cancelled departures (no crew): %d\n", cancelledFlights);
    if (departureCount > 0) {
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Departure Success Rate: %.1f%%\n", (float)successfulFlights / departureCount * 100.0f);
    } else {
         pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Departure Success Rate: N/A (0 departures)\n");
    }

end_process_slot_legacy:
     if (pos >= sizeof(temp_buffer) - 1) {
        const char *trunc_msg = "... [Output Truncated]\n";
        int space_for_trunc = strlen(trunc_msg) + 1;
        if (pos > sizeof(temp_buffer) - space_for_trunc) {
            pos = sizeof(temp_buffer) - space_for_trunc;
        }
        strcpy(temp_buffer + pos, trunc_msg);
        pos += strlen(trunc_msg);
    }
    temp_buffer[pos] = '\0'; // Ensure null termination
    appendToOutput(output, max_len, temp_buffer, NULL);
    free(temp_records); // Free the temporary copy
}


// Display Flight Details as a Table (Uses modified drawTableCell)
void displayFlightDetails(AirlineRecord records[], int count, char output[], int* line_count, float* scroll_offset, int x, int y, int width, int height) {
    // This function now primarily manages the scrolling and calls drawTableCell.
    if (count <= 0) {
        DrawText("No flight data loaded.", x + PADDING, y + PADDING, BASE_FONT_SIZE, COLOR_TEXT_MEDIUM);
        return;
    }

    int header_font_size = BASE_FONT_SIZE;
    int cell_font_size = BASE_FONT_SIZE - 2;
    int cell_height = 35;
    int column_widths[] = { 50, 160, 110, 100, 130, 130, 130, 60, 90, 70 }; // ID, Airline, Domain, Type, From, To, Via, Slot, Emerc, Delay
    int num_columns = sizeof(column_widths) / sizeof(column_widths[0]);
    int total_content_width = 0;
    for (int i = 0; i < num_columns; i++) total_content_width += column_widths[i];

    // Calculate total height for scrollbar (content height)
    *line_count = count + 1; // Update line count for potential external use

    // Sort records by final slot for display
    qsort(records, count, sizeof(AirlineRecord), compareBySlot);

    BeginScissorMode(x, y, width, height);

    // Header Row
    const char* headers[] = {"ID", "Airline", "Domain", "Type", "From", "To", "Via", "Slot", "Emerc", "Delay"};
    int current_x = x;
    int current_y_draw = y + (int)(*scroll_offset); // Apply scroll offset for drawing

    for (int i = 0; i < num_columns; i++) {
        drawTableCell(headers[i], current_x, current_y_draw, column_widths[i], cell_height, header_font_size, COLOR_TEXT_DARK, true, false, false);
        current_x += column_widths[i];
    }
    current_y_draw += cell_height;

    // Data rows
    Vector2 mouse_pos = GetMousePosition();
    for (int i = 0; i < count; i++) {
         // Only draw if visible within the scissor rectangle
        if (current_y_draw + cell_height < y || current_y_draw > y + height) {
             current_y_draw += cell_height;
             continue;
        }

        char id_str[16], slot_str[16], delay_str[16], emerg_str[8];
        snprintf(id_str, sizeof(id_str), "%d", i + 1);
        // Display slot status correctly based on final assigned slot
        if (records[i].slot_order == 0) snprintf(slot_str, sizeof(slot_str), "N/A");
        else if (records[i].slot_order == -1) snprintf(slot_str, sizeof(slot_str), "RRT");
        else if (records[i].slot_order == -2) snprintf(slot_str, sizeof(slot_str), "UNC");
        else snprintf(slot_str, sizeof(slot_str), "%d", records[i].slot_order);
        snprintf(delay_str, sizeof(delay_str), "%d", records[i].delay_minutes);
        snprintf(emerg_str, sizeof(emerg_str), "%s", records[i].emergency ? "YES" : "No");


        const char* row_data[] = {
            id_str,
            records[i].airline,
            records[i].plane_domain,
            records[i].arr_or_dip == 0 ? "Departure" : "Arrival",
            records[i].starting_destination,
            records[i].end_destination,
            records[i].central_destination[0] != '\0' ? records[i].central_destination : "-", // Show '-' if empty
            slot_str,
            emerg_str,
            delay_str
        };

        current_x = x;
        // Determine row hover based on the entire row's rectangle
        bool row_hovered = CheckCollisionPointRec(mouse_pos, (Rectangle){(float)x, (float)current_y_draw, (float)total_content_width, (float)cell_height});

        for (int j = 0; j < num_columns; j++) {
            // Use row_hovered state for consistent highlighting across the row
            drawTableCell(row_data[j], current_x, current_y_draw, column_widths[j], cell_height, cell_font_size, COLOR_TEXT_DARK, false, row_hovered, records[i].emergency);
            current_x += column_widths[j];
        }
        current_y_draw += cell_height;
    }

    EndScissorMode();
}


// Initialize Slots
void initializeSlots(SlotStatus slots[], int num_slots, int max_capacity) {
    if (!slots || num_slots <= 0) return;
    for (int i = 0; i < num_slots; i++) {
        slots[i].slot_number = i + 1;
        slots[i].planes_allocated = 0;
        slots[i].international_count = 0;
        slots[i].domestic_count = 0;
        slots[i].max_capacity = max_capacity > 0 ? max_capacity : MAX_CAPACITY_PER_SLOT; // Use default if invalid
        for (int r = 0; r < NUM_RUNWAYS; r++) {
            slots[i].runway_assignments[r] = 0;
        }
    }
}

// Print Slot Status (Graphical Table)
void printSlotStatus(SlotStatus slots[], int num_slots, char output[], int* line_count, float* scroll_offset, int x, int y, int width, int height) {
    // This function now primarily manages the scrolling and calls drawTableCell.
    if (slots == NULL || num_slots <= 0) {
        DrawText("No slot data available.", x + PADDING, y + PADDING, BASE_FONT_SIZE, COLOR_TEXT_MEDIUM);
        return;
    }

    int header_font_size = BASE_FONT_SIZE;
    int cell_font_size = BASE_FONT_SIZE - 2;
    int cell_height = 35;
    int column_widths[] = { 60, 100, 110, 100, 100, 80, 80, 80 }; // Slot, Total, Intl, Dom, Load, R1, R2, R3
    int num_columns = sizeof(column_widths) / sizeof(column_widths[0]);
    int total_content_width = 0;
    for (int i = 0; i < num_columns; i++) total_content_width += column_widths[i];

    // Calculate total height for scrollbar
    *line_count = num_slots + 1; // Update line count

    BeginScissorMode(x, y, width, height);

    // Header Row
    const char* headers[] = {"Slot", "Total", "Int'l", "Dom", "Load", "Rwy 1", "Rwy 2", "Rwy 3"};
    int current_x = x;
    int current_y_draw = y + (int)(*scroll_offset); // Use separate drawing y

    for (int i = 0; i < num_columns; i++) {
        drawTableCell(headers[i], current_x, current_y_draw, column_widths[i], cell_height, header_font_size, COLOR_TEXT_DARK, true, false, false);
        current_x += column_widths[i];
    }
    current_y_draw += cell_height;

    // Data rows
    Vector2 mouse_pos = GetMousePosition();
    for (int i = 0; i < num_slots; i++) {
         // Only draw if visible
        if (current_y_draw + cell_height < y || current_y_draw > y + height) {
             current_y_draw += cell_height;
             continue;
        }

        char slot_str[16], total_str[16], intl_str[16], dom_str[16], load_str[16];
        char rwy1_str[16], rwy2_str[16], rwy3_str[16];
        snprintf(slot_str, sizeof(slot_str), "%d", slots[i].slot_number);
        snprintf(total_str, sizeof(total_str), "%d", slots[i].planes_allocated);
        snprintf(intl_str, sizeof(intl_str), "%d", slots[i].international_count);
        snprintf(dom_str, sizeof(dom_str), "%d", slots[i].domestic_count);
        snprintf(load_str, sizeof(load_str), "%d/%d", slots[i].planes_allocated, slots[i].max_capacity); // Show allocated/max for Load
        snprintf(rwy1_str, sizeof(rwy1_str), "%d", slots[i].runway_assignments[0]);
        snprintf(rwy2_str, sizeof(rwy2_str), "%d", slots[i].runway_assignments[1]);
        snprintf(rwy3_str, sizeof(rwy3_str), "%d", slots[i].runway_assignments[2]);


        const char* row_data[] = {slot_str, total_str, intl_str, dom_str, load_str, rwy1_str, rwy2_str, rwy3_str};

        current_x = x;
        bool row_hovered = CheckCollisionPointRec(mouse_pos, (Rectangle){(float)x, (float)current_y_draw, (float)total_content_width, (float)cell_height});

        for (int j = 0; j < num_columns; j++) {
            drawTableCell(row_data[j], current_x, current_y_draw, column_widths[j], cell_height, cell_font_size, COLOR_TEXT_DARK, false, row_hovered, false);
            current_x += column_widths[j];
        }
        current_y_draw += cell_height;
    }

    EndScissorMode();
}

// Compare Flight Priority (Emergency -> International -> Domestic)
int compareFlightPriority(const void *a, const void *b) {
    // Used for sorting arrays of AirlineRecord* pointers
    const AirlineRecord *flightA = *(const AirlineRecord **)a;
    const AirlineRecord *flightB = *(const AirlineRecord **)b;

    // Emergency first
    if (flightA->emergency != flightB->emergency) {
        return flightB->emergency - flightA->emergency; // Higher emergency (1) comes before lower (0)
    }

    // Then International before Domestic
    int aIntl = (strcmp(flightA->plane_domain, "international") == 0);
    int bIntl = (strcmp(flightB->plane_domain, "international") == 0);
    if (aIntl != bIntl) {
        return bIntl - aIntl; // International (1) comes before Domestic (0)
    }

    return 0; // Keep original relative order if priorities are equal
}


// Slot Allocator (Assigns flights to slots based on priority)
void slotAllocator(AirlineRecord records[], int count, SlotStatus slots[], int num_slots) {
    if (records == NULL || slots == NULL || count <= 0 || num_slots <= 0) return;

    // 1. Initialize slots
    initializeSlots(slots, num_slots, MAX_CAPACITY_PER_SLOT);

    // 2. Process flights: Assign initial slot_order based on logic
    int emergency_unallocated = 0;
    int non_emergency_count = 0;
    AirlineRecord *non_emergency_flights[MAX_RECORDS]; // Array of pointers

    for (int i = 0; i < count; i++) {
        records[i].runway_assignment = 0; // Reset runway assignment here
        int requested_slot = records[i].slot_order; // Get the requested slot from CSV

        if (records[i].emergency) {
            int requested_slot_idx = -1;
            // Find index for requested slot number (1-based)
            for (int s=0; s < num_slots; ++s) {
                if (slots[s].slot_number == requested_slot) {
                    requested_slot_idx = s;
                    break;
                }
            }

            // Check if valid slot requested and has space
            if (requested_slot_idx != -1 && slots[requested_slot_idx].planes_allocated < slots[requested_slot_idx].max_capacity) {
                // Assign to requested slot - slot_order remains requested_slot
                slots[requested_slot_idx].planes_allocated++;
                if (strcmp(records[i].plane_domain, "international") == 0) slots[requested_slot_idx].international_count++;
                else slots[requested_slot_idx].domestic_count++;
            } else {
                // Cannot place in requested slot. Try earliest available.
                int assigned_slot_idx = -1;
                for (int s = 0; s < num_slots; s++) {
                    if (slots[s].planes_allocated < slots[s].max_capacity) {
                        assigned_slot_idx = s;
                        break;
                    }
                }
                if (assigned_slot_idx != -1) {
                    records[i].slot_order = slots[assigned_slot_idx].slot_number; // Overwrite requested with assigned
                    slots[assigned_slot_idx].planes_allocated++;
                    if (strcmp(records[i].plane_domain, "international") == 0) slots[assigned_slot_idx].international_count++;
                    else slots[assigned_slot_idx].domestic_count++;
                } else {
                    records[i].slot_order = 0; // Mark as unallocated
                    emergency_unallocated++;
                }
            }
        } else {
            // Non-emergency: Reset slot_order to 0 for now, add to list for sorting
            records[i].slot_order = 0; // Mark as needing allocation
            if (non_emergency_count < MAX_RECORDS) {
                non_emergency_flights[non_emergency_count++] = &records[i];
            } else {
                 fprintf(stderr, "Warning: Exceeded non-emergency flight capacity in slotAllocator.\n");
            }
        }
    }
    if (emergency_unallocated > 0) {
         fprintf(stderr, "Warning: %d emergency flight(s) could not be allocated any slot.\n", emergency_unallocated);
    }


    // 3. Sort non-emergency flights by priority
    qsort(non_emergency_flights, non_emergency_count, sizeof(AirlineRecord*), compareFlightPriority);

    // 4. Allocate sorted non-emergency flights sequentially
    int non_emergency_unallocated = 0;
    for (int i = 0; i < non_emergency_count; i++) {
        AirlineRecord *flight = non_emergency_flights[i];
        int assigned_slot_idx = -1;

        // Find the next slot with available capacity
        for (int s = 0; s < num_slots; s++) {
             if (slots[s].planes_allocated < slots[s].max_capacity) {
                 assigned_slot_idx = s;
                 break;
             }
        }

        if (assigned_slot_idx != -1) {
            flight->slot_order = slots[assigned_slot_idx].slot_number; // Assign slot number
            slots[assigned_slot_idx].planes_allocated++;
            if (strcmp(flight->plane_domain, "international") == 0) slots[assigned_slot_idx].international_count++;
            else slots[assigned_slot_idx].domestic_count++;
        } else {
            flight->slot_order = 0; // Remains unallocated (already 0)
            non_emergency_unallocated++;
        }
    }
     if (non_emergency_unallocated > 0) {
          // Use appendToOutput or similar if logging to main UI is desired
          // fprintf(stderr, "Info: %d non-emergency flight(s) could not be allocated a slot.\n", non_emergency_unallocated);
     }
}


// Process Flights With Slot Allocation (Uses assigned slots)
void processFlightsWithSlotAllocation(AirlineRecord records[], int count, CrewTeam domestic[], CrewTeam international[], CrewTeam backup[], char output[], int max_len) {
    char temp_buffer[8192] = {0};
    int pos = 0;
    int successfulDepartures = 0;
    int cancelledDepartures = 0;
    int unprocessedFlights = 0; // Flights not processed (unalloc, rerouted etc.)
    int departureCount = 0;     // Departures needing crew (that were allocated a slot > 0)
    int arrivalCount = 0;       // Arrivals (allocated a slot > 0)

    // Initial count of processable flights
    for (int i = 0; i < count; i++) {
        if (records[i].slot_order <= 0) { // Includes 0, -1, -2 etc.
             unprocessedFlights++;
        } else if (records[i].arr_or_dip == 0) { // Departure allocated
            departureCount++;
        } else { // Arrival allocated
            arrivalCount++;
        }
    }
    int allocatedFlights = count - unprocessedFlights;

    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Processing %d allocated flights (%d departures, %d arrivals).\n%d flights not processed (unallocated/rerouted/etc.).\n",
           allocatedFlights, departureCount, arrivalCount, unprocessedFlights);

    // Sort by the *assigned* slot order (using the enhanced compareBySlot)
    qsort(records, count, sizeof(AirlineRecord), compareBySlot);

    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n--- Processing Flights by Assigned Slot ---\n");
    if (pos >= sizeof(temp_buffer) - 200) goto end_process_alloc;

    for (int currentSlot = 1; currentSlot <= NUM_SLOTS; currentSlot++) {
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n=== Processing Slot %d ===\n", currentSlot);
        if (pos >= sizeof(temp_buffer) - 200) goto end_process_alloc;

        updateCrewStatus(domestic, international, backup); // Update crew at start of slot

        bool processed_in_slot = false;
        for (int i = 0; i < count; i++) {
            // Only process flights assigned to this specific, positive slot
            if (records[i].slot_order == currentSlot) {
                 processed_in_slot = true;
                 bool isInternational = (strcmp(records[i].plane_domain, "international") == 0);

                if (records[i].arr_or_dip == 0) { // Departure
                    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "[%s] DEP: %s (%s)",
                                   records[i].emergency ? "EMERG" : "Norml",
                                   records[i].plane_id, records[i].airline);
                    if (pos >= sizeof(temp_buffer) - 200) goto end_process_alloc;

                    int crewResult = allocateCrew(isInternational, domestic, international, backup);
                    if (crewResult != 0) {
                        successfulDepartures++;
                         pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, " -> Crew OK (Team %c%d)\n",
                                        crewResult < 0 ? 'B' : (isInternational ? 'I' : 'D'), abs(crewResult));
                    } else {
                        cancelledDepartures++;
                        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, " -> Crew FAILED -> CANCELLED\n");
                    }
                    if (pos >= sizeof(temp_buffer) - 200) goto end_process_alloc;

                } else { // Arrival
                    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "[%s] ARR: %s (%s) -> OK (No Crew Needed)\n",
                                   records[i].emergency ? "EMERG" : "Norml",
                                   records[i].plane_id, records[i].airline);
                    if (pos >= sizeof(temp_buffer) - 200) goto end_process_alloc;
                }
            }
        }
         if (!processed_in_slot) {
             pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "(No flights processed in this slot)\n");
             if (pos >= sizeof(temp_buffer) - 200) goto end_process_alloc;
         }
    }

    // Summary
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n--- Flight Operations Summary (Assigned Slots) ---\n");
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Total flights initially: %d\n", count);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Flights allocated a slot: %d\n", allocatedFlights);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Flights not processed: %d\n", unprocessedFlights);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Allocated departures needing crew: %d\n", departureCount);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Successful departures (crew allocated): %d\n", successfulDepartures);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Cancelled departures (no crew): %d\n", cancelledDepartures);
    if (departureCount > 0) {
         pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Departure Success Rate (of allocated): %.1f%%\n", (float)successfulDepartures / departureCount * 100.0f);
    } else {
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "Departure Success Rate: N/A (0 allocated departures)\n");
    }
     if (pos >= sizeof(temp_buffer) - 1) goto end_process_alloc; // Check before final null termination

end_process_alloc:
    if (pos >= sizeof(temp_buffer) - 1) {
       const char *trunc_msg = "... [Output Truncated]\n";
       int space_left = sizeof(temp_buffer) - pos -1;
       int needed = strlen(trunc_msg) + 1;
       if (space_left < needed) pos -= (needed - space_left); // Overwrite if necessary
       strcpy(temp_buffer + pos, trunc_msg);
       pos += strlen(trunc_msg);
    }
    temp_buffer[pos] = '\0';
    appendToOutput(output, max_len, temp_buffer, NULL);
}


// Runway Planner (Assigns flights within a slot to runways)
void runwayPlanner(AirlineRecord records[], int count, SlotStatus slots[], int num_slots) {
     if (!records || !slots || count <= 0 || num_slots <= 0) return;

     // Reset previous runway assignments in slot status and applicable records
     for (int s = 0; s < num_slots; s++) {
         for (int r = 0; r < NUM_RUNWAYS; r++) {
             slots[s].runway_assignments[r] = 0;
         }
     }
      for (int i = 0; i < count; i++) {
         // Only reset runway if flight is actually assigned a valid slot (>0)
         if (records[i].slot_order > 0) {
             records[i].runway_assignment = 0;
         } else {
             records[i].runway_assignment = 0; // Ensure non-positive slot flights have no runway
         }
     }


    for (int slot_idx = 0; slot_idx < num_slots; slot_idx++) {
        int current_slot_num = slots[slot_idx].slot_number;

        // 1. Collect pointers to flights assigned to the current slot
        AirlineRecord *slot_flights[MAX_RECORDS];
        int planes_in_slot = 0;
        for (int i = 0; i < count; i++) {
            if (records[i].slot_order == current_slot_num) { // Only consider positively assigned slots
                if (planes_in_slot < MAX_RECORDS) {
                    slot_flights[planes_in_slot++] = &records[i];
                } else {
                    fprintf(stderr, "Warning: Exceeded MAX_RECORDS capacity for slot %d in runwayPlanner.\n", current_slot_num);
                    break;
                }
            }
        }

        if (planes_in_slot == 0) continue; // Skip empty slots

        // 2. Sort flights within the slot by priority
         qsort(slot_flights, planes_in_slot, sizeof(AirlineRecord*), compareFlightPriority);

        // 3. Assign runways, balancing load
        int runway_counts[NUM_RUNWAYS] = {0}; // Track planes assigned per runway IN THIS SLOT

        for (int i = 0; i < planes_in_slot; i++) {
            AirlineRecord *flight = slot_flights[i];
            int target_runway_idx = -1;

            // Find the runway with the fewest assignments *so far in this slot*
            int min_count = MAX_PER_RUNWAY + 1;
            int best_runway = 0;
            for (int r = 0; r < NUM_RUNWAYS; r++) {
                if (runway_counts[r] < min_count) {
                    min_count = runway_counts[r];
                    best_runway = r;
                }
            }
            target_runway_idx = best_runway;

            // Check if the chosen runway is full for this slot
            if (runway_counts[target_runway_idx] < MAX_PER_RUNWAY) {
                 flight->runway_assignment = target_runway_idx + 1; // Assign 1-based runway number
                 runway_counts[target_runway_idx]++;
                 slots[slot_idx].runway_assignments[target_runway_idx]++; // Update slot status counter
            } else {
                 flight->runway_assignment = 0; // Mark as unassigned to a runway
                 // fprintf(stderr, "Info: Could not assign runway in slot %d for flight %s (Runways full)\n", current_slot_num, flight->plane_id);
            }
        }
    }
}


// Display Runway Allocation Plan (Graphical)
void displayRunwayAllocationPlan(AirlineRecord records[], int count, SlotStatus slots[], int num_slots, char output[], int max_len, int x, int y, int width, int height, float* scroll_offset) {
    if (count <= 0 || num_slots <= 0 || !records || !slots) {
        DrawText("No runway allocation data available.", x + PADDING, y + PADDING, BASE_FONT_SIZE, COLOR_TEXT_MEDIUM);
        return;
    }

    int title_font_size = BASE_FONT_SIZE + 2;
    int header_font_size = BASE_FONT_SIZE;
    int cell_font_size = BASE_FONT_SIZE - 3;
    int cell_height = 30;
    int slot_header_height = 40;
    int section_spacing = 15;

    int runway_col_width = (width - PADDING * 2) / NUM_RUNWAYS;
    if (runway_col_width < 100) runway_col_width = 100;
    int total_content_width = runway_col_width * NUM_RUNWAYS;

    // Calculate total content height for scrolling (Corrected logic)
    int total_height_content = 0;
    for (int slot_idx = 0; slot_idx < num_slots; slot_idx++) {
         total_height_content += slot_header_height + cell_height; // Slot title + Runway headers
         int max_flights_in_runway = 0;
         for(int r=0; r<NUM_RUNWAYS; ++r) {
             if (slots[slot_idx].runway_assignments[r] > max_flights_in_runway) {
                 max_flights_in_runway = slots[slot_idx].runway_assignments[r];
             }
         }
         if (max_flights_in_runway == 0) max_flights_in_runway = 1; // Min 1 row
         total_height_content += max_flights_in_runway * cell_height;
         total_height_content += section_spacing;
    }

    BeginScissorMode(x, y, width, height);

    int current_y_draw = y + (int)(*scroll_offset);
    Vector2 mouse_pos = GetMousePosition();

    for (int slot_idx = 0; slot_idx < num_slots; slot_idx++) {
        int current_slot_num = slots[slot_idx].slot_number;

        // --- Draw Slot Header ---
        Rectangle slot_header_rect = {(float)x, (float)current_y_draw, (float)total_content_width, (float)slot_header_height};
        if (slot_header_rect.y + slot_header_rect.height > y && slot_header_rect.y < y + height) { // Culling
             DrawRectangleRec(slot_header_rect, COLOR_HEADER_BG);
             DrawRectangleLinesEx(slot_header_rect, 1.0f, COLOR_BORDER);
             char header_text[64];
             snprintf(header_text, sizeof(header_text), "Time Slot %d (Total: %d)", current_slot_num, slots[slot_idx].planes_allocated);
             Vector2 title_size = MeasureTextEx(GetFontDefault(), header_text, title_font_size, 1.0f);
             DrawText(header_text, x + PADDING, current_y_draw + (slot_header_height - title_size.y) / 2, title_font_size, COLOR_TEXT_DARK);
        }
        current_y_draw += slot_header_height;


        // --- Draw Runway Headers ---
         Rectangle runway_header_rect = {(float)x, (float)current_y_draw, (float)total_content_width, (float)cell_height};
         if (runway_header_rect.y + runway_header_rect.height > y && runway_header_rect.y < y + height) { // Culling
             int current_x = x;
             for (int r = 0; r < NUM_RUNWAYS; r++) {
                 char r_header[32];
                 snprintf(r_header, sizeof(r_header), "Runway %d (%d)", r + 1, slots[slot_idx].runway_assignments[r]);
                 drawTableCell(r_header, current_x, current_y_draw, runway_col_width, cell_height, header_font_size, COLOR_TEXT_DARK, true, false, false);
                 current_x += runway_col_width;
             }
         }
        current_y_draw += cell_height;


        // --- Prepare Data for this Slot's Runways ---
        AirlineRecord *flights_in_slot[MAX_RECORDS];
        int slot_flight_count = 0;
        for(int i=0; i<count; ++i) {
            if (records[i].slot_order == current_slot_num && slot_flight_count < MAX_RECORDS) {
                flights_in_slot[slot_flight_count++] = &records[i];
            }
        }

        int max_rows_needed = 0;
        for(int r=0; r<NUM_RUNWAYS; ++r) {
             if (slots[slot_idx].runway_assignments[r] > max_rows_needed) {
                 max_rows_needed = slots[slot_idx].runway_assignments[r];
             }
         }
         if (max_rows_needed == 0) max_rows_needed = 1;
         // Clamp for drawing loop safety if runway count exceeds limit (shouldn't happen with MAX_PER_RUNWAY check in planner)
         if (max_rows_needed > MAX_PER_RUNWAY * 2) max_rows_needed = MAX_PER_RUNWAY * 2; // Arbitrary safe upper limit for drawing


        // --- Draw Flight Cells ---
        for (int row = 0; row < max_rows_needed; row++) {
             Rectangle row_rect = {(float)x, (float)current_y_draw, (float)total_content_width, (float)cell_height};
             if (row_rect.y + row_rect.height < y || row_rect.y > y + height) { // Culling
                 current_y_draw += cell_height;
                 continue;
             }

            int current_x = x;
            for (int r = 0; r < NUM_RUNWAYS; r++) { // Iterate through runways (columns)
                char cell_text[256] = "";
                bool is_emergency = false;
                bool cell_hovered = false;
                int current_runway_num = r + 1;
                int flight_index_in_runway = 0; // Which flight this is for the current runway (0th, 1st, etc.)

                // Find the flight that belongs in this cell (runway r, row number row)
                AirlineRecord *flight_for_cell = NULL;
                for(int k=0; k < slot_flight_count; ++k) {
                     // Check if the flight is assigned to the current runway column
                    if (flights_in_slot[k]->runway_assignment == current_runway_num) {
                        // If this is the correct 'row'-th flight for this runway, assign it
                        if (flight_index_in_runway == row) {
                             flight_for_cell = flights_in_slot[k];
                             break; // Found the flight for this cell
                        }
                        flight_index_in_runway++; // Increment count for this runway
                    }
                }

                if (flight_for_cell != NULL) {
                    snprintf(cell_text, sizeof(cell_text), "%s (%c) %dm %s",
                             flight_for_cell->plane_id,
                             flight_for_cell->arr_or_dip == 0 ? 'D' : 'A',
                             flight_for_cell->delay_minutes,
                             flight_for_cell->emergency ? "[E]" : "");
                     is_emergency = flight_for_cell->emergency;
                     cell_hovered = CheckCollisionPointRec(mouse_pos, (Rectangle){(float)current_x, (float)current_y_draw, (float)runway_col_width, (float)cell_height});
                }

                drawTableCell(cell_text, current_x, current_y_draw, runway_col_width, cell_height, cell_font_size, COLOR_TEXT_DARK, false, cell_hovered, is_emergency);
                current_x += runway_col_width;
            }
            current_y_draw += cell_height;
        }
        current_y_draw += section_spacing; // Add space between slots
    }

    EndScissorMode();
}


// Airplane Delay (Assigns random delays)
void airplaneDelay(AirlineRecord records[], int count, SlotStatus slots[], int num_slots, char output[], int max_len) {
    char temp_buffer[8192] = {0};
    int pos = 0;
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n--- Assigning Flight Delays ---\n");
    if (pos >= sizeof(temp_buffer) - 100) goto end_delay;

    const char *delay_reasons[] = {
        "Weather", "ATC Flow", "Technical", "Crew Duty", "Late Aircraft", "Security", "Ops"
    };
    int num_reasons = sizeof(delay_reasons) / sizeof(delay_reasons[0]);
    int delay_applied_count = 0;

    srand(time(NULL)); // Seed random number generator

    for (int i = 0; i < count; i++) {
         records[i].delay_minutes = 0; // Reset delay first

         // Assign delay only if flight is allocated to a valid slot (>0)
         if (records[i].slot_order > 0) {
             int chance = rand() % 100; // 0-99

             // Define delay probabilities and ranges
             if (chance < 30) { // 30% chance of delay
                 delay_applied_count++;
                 if (chance < 21) { // 70% of delays are Minor (0-20 -> 21 numbers)
                     records[i].delay_minutes = (rand() % 15) + 1; // 1-15 min
                 } else if (chance < 27) { // 20% of delays are Medium (21-26 -> 6 numbers)
                     records[i].delay_minutes = (rand() % 15) + 16; // 16-30 min
                 } else { // 10% of delays are Severe (27-29 -> 3 numbers)
                     records[i].delay_minutes = (rand() % 30) + 31; // 31-60 min
                 }

                 // Log the delay assignment
                 if (records[i].delay_minutes > 0 && pos < sizeof(temp_buffer) - 200) {
                     int reason_idx = rand() % num_reasons;
                     pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos,
                                     "  Flight %s (%s) delayed by %d min. Reason: %s\n",
                                     records[i].plane_id,
                                     records[i].arr_or_dip == 0 ? "DEP" : "ARR",
                                     records[i].delay_minutes,
                                     delay_reasons[reason_idx]);
                 }
             }
         }
    }

    // Delay Statistics Calculation
    int no_delay = 0, minor_delay = 0, medium_delay = 0, severe_delay = 0;
    int allocated_count_stats = 0;
    for (int i = 0; i < count; i++) {
        if (records[i].slot_order > 0) { // Only count stats for positively allocated flights
            allocated_count_stats++;
            if (records[i].delay_minutes == 0) no_delay++;
            else if (records[i].delay_minutes <= 15) minor_delay++;
            else if (records[i].delay_minutes <= 30) medium_delay++;
            else severe_delay++;
        }
    }

    // Print Statistics
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n--- Delay Statistics (for %d allocated flights) ---\n", allocated_count_stats);
    if (pos >= sizeof(temp_buffer) - 300) goto end_delay;

    if (allocated_count_stats > 0) {
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  No Delay:      %d (%.1f%%)\n", no_delay, (float)no_delay / allocated_count_stats * 100.0f);
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Minor (1-15m): %d (%.1f%%)\n", minor_delay, (float)minor_delay / allocated_count_stats * 100.0f);
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Medium (16-30m):%d (%.1f%%)\n", medium_delay, (float)medium_delay / allocated_count_stats * 100.0f);
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Severe (>30m): %d (%.1f%%)\n", severe_delay, (float)severe_delay / allocated_count_stats * 100.0f);
    } else {
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  No allocated flights to calculate statistics.\n");
    }

end_delay:
     if (pos >= sizeof(temp_buffer) - 1) {
        const char *trunc_msg = "... [Delay Log Truncated]\n";
        int space_left = sizeof(temp_buffer) - pos -1;
        int needed = strlen(trunc_msg) + 1;
        if (space_left < needed) pos -= (needed - space_left);
        strcpy(temp_buffer + pos, trunc_msg);
        pos += strlen(trunc_msg);
    }
    temp_buffer[pos] = '\0';
    appendToOutput(output, max_len, temp_buffer, NULL);
}


// Simulate Slot With Delays (Handles conflicts and pushes flights)
void simulateSlotWithDelays(AirlineRecord records[], int count, SlotStatus slots[], int num_slots, char output[], int max_len) {
    char temp_buffer[12288] = {0};
    int pos = 0;
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n--- Live Delay Simulation ---\n");
    if (pos >= sizeof(temp_buffer) - 100) goto end_sim;

    // Create temporary copies for simulation state
    AirlineRecord *sim_records = malloc(count * sizeof(AirlineRecord));
    if (!sim_records) {
        appendToOutput(output, max_len, "ERROR: Failed to allocate memory for simulation records.\n", NULL);
        goto end_sim;
    }
    memcpy(sim_records, records, count * sizeof(AirlineRecord));

     SlotStatus *sim_slots = malloc(num_slots * sizeof(SlotStatus));
     if (!sim_slots) {
         free(sim_records);
         appendToOutput(output, max_len, "ERROR: Failed to allocate memory for slot simulation status.\n", NULL);
         goto end_sim;
     }
     memcpy(sim_slots, slots, num_slots * sizeof(SlotStatus));


    int rerouted_count = 0;
    int moved_to_next_slot_count = 0;
    int could_not_move_count = 0;


    for (int slot_idx = 0; slot_idx < num_slots; slot_idx++) {
        int current_slot_num = sim_slots[slot_idx].slot_number;
        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n=== Simulating Slot %d ===\n", current_slot_num);
        if (pos >= sizeof(temp_buffer) - 200) goto end_sim_loop;

        // 1. Collect pointers to flights currently assigned to this slot in sim_records
        AirlineRecord *slot_flights[MAX_RECORDS];
        int planes_in_slot = 0;
        for (int i = 0; i < count; i++) {
            if (sim_records[i].slot_order == current_slot_num) {
                 if (planes_in_slot < MAX_RECORDS) {
                     slot_flights[planes_in_slot++] = &sim_records[i];
                 } else {
                      fprintf(stderr, "Warning: Exceeded MAX_RECORDS capacity for slot %d in simulateSlotWithDelays.\n", current_slot_num);
                      break;
                 }
            }
        }

        if (planes_in_slot == 0) {
            pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  (No flights currently in this slot)\n");
             if (pos >= sizeof(temp_buffer) - 200) goto end_sim_loop;
            continue;
        }

        // 2. Sort flights within the slot by priority
        qsort(slot_flights, planes_in_slot, sizeof(AirlineRecord*), compareFlightPriority);

        // 3. Simulate runway usage
        int runway_finish_time[NUM_RUNWAYS] = {0};
        int current_buffer_time = NORMAL_BUFFER_TIME;
        bool buffer_reduced = false;

        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Runway Capacity: %d/runway | Initial Buffer: %d min\n", MAX_PER_RUNWAY, current_buffer_time);
        if (pos >= sizeof(temp_buffer) - 200) goto end_sim_loop;


        for (int i = 0; i < planes_in_slot; i++) {
            AirlineRecord *flight = slot_flights[i];
            // Ensure flight has a valid runway assigned from previous planning step
            if (flight->runway_assignment <= 0 || flight->runway_assignment > NUM_RUNWAYS) {
                 pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  * Flight %s has invalid runway assignment (%d) in slot %d - skipping simulation.\n", flight->plane_id, flight->runway_assignment, current_slot_num);
                 if (pos >= sizeof(temp_buffer) - 200) goto end_sim_loop;
                 flight->slot_order = -2; // Mark as unaccommodated due to planning issue
                 flight->runway_assignment = -2;
                 // Adjust counts in sim_slots if needed, though it was counted initially
                 sim_slots[slot_idx].planes_allocated--; // Assume it was counted, now remove
                 if (strcmp(flight->plane_domain, "international") == 0) sim_slots[slot_idx].international_count--;
                 else sim_slots[slot_idx].domestic_count--;
                 could_not_move_count++; // Count this as unaccommodated
                 continue;
            }
            int runway_idx = flight->runway_assignment - 1; // 0-based


            // --- Severe Delay Handling ---
            if (flight->delay_minutes > 30) {
                pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  ! Flight %s (%s) severely delayed (%d min) -> REROUTED\n",
                                flight->plane_id, flight->arr_or_dip == 0 ? "DEP" : "ARR", flight->delay_minutes);
                rerouted_count++;

                // Update sim counts and mark flight as out of system
                sim_slots[slot_idx].planes_allocated--;
                 // Decrement the correct runway count
                 sim_slots[slot_idx].runway_assignments[runway_idx]--;
                if (strcmp(flight->plane_domain, "international") == 0) sim_slots[slot_idx].international_count--;
                else sim_slots[slot_idx].domestic_count--;

                flight->slot_order = -1; // Mark as rerouted
                flight->runway_assignment = -1;

                if (pos >= sizeof(temp_buffer) - 200) goto end_sim_loop;
                continue;
            }

            // --- Simulation Logic ---
            int start_time = runway_finish_time[runway_idx];
            int required_time = start_time + flight->delay_minutes + current_buffer_time;

            // Check if it fits with current buffer
            if (required_time <= SLOT_DURATION) {
                pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  > Flight %s (%s) on Rwy %d. Start: %d, Delay: %d, Finish: %d (Buffer: %d)\n",
                                flight->plane_id, flight->arr_or_dip == 0 ? "DEP" : "ARR", runway_idx + 1,
                                start_time, flight->delay_minutes, required_time, current_buffer_time);
                runway_finish_time[runway_idx] = required_time;
                 if (pos >= sizeof(temp_buffer) - 200) goto end_sim_loop;

            } else {
                // Try with reduced buffer
                if (!buffer_reduced) {
                     current_buffer_time = REDUCED_BUFFER_TIME;
                     buffer_reduced = true;
                     pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  * Reducing buffer time to %d minutes for remaining flights in slot %d.\n", current_buffer_time, current_slot_num);
                     if (pos >= sizeof(temp_buffer) - 200) goto end_sim_loop;
                }
                required_time = start_time + flight->delay_minutes + current_buffer_time;

                if (required_time <= SLOT_DURATION) {
                    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  > Flight %s (%s) on Rwy %d. Start: %d, Delay: %d, Finish: %d (Buffer: %d*)\n",
                                    flight->plane_id, flight->arr_or_dip == 0 ? "DEP" : "ARR", runway_idx + 1,
                                    start_time, flight->delay_minutes, required_time, current_buffer_time);
                     runway_finish_time[runway_idx] = required_time;
                     if (pos >= sizeof(temp_buffer) - 200) goto end_sim_loop;

                } else {
                    // Cannot fit, try moving to next slot
                    int next_slot_idx = slot_idx + 1;
                    if (next_slot_idx < num_slots && sim_slots[next_slot_idx].planes_allocated < sim_slots[next_slot_idx].max_capacity) {
                         pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  ! Flight %s (%s) delayed (%d min) -> MOVED to Slot %d\n",
                                         flight->plane_id, flight->arr_or_dip == 0 ? "DEP" : "ARR",
                                         flight->delay_minutes, sim_slots[next_slot_idx].slot_number);
                         moved_to_next_slot_count++;

                         // Update counts for both slots in simulation status
                         sim_slots[slot_idx].planes_allocated--;
                         sim_slots[slot_idx].runway_assignments[runway_idx]--; // Decrement original runway count
                         if (strcmp(flight->plane_domain, "international") == 0) sim_slots[slot_idx].international_count--;
                         else sim_slots[slot_idx].domestic_count--;

                         sim_slots[next_slot_idx].planes_allocated++;
                         if (strcmp(flight->plane_domain, "international") == 0) sim_slots[next_slot_idx].international_count++;
                         else sim_slots[next_slot_idx].domestic_count++;

                         // Update flight's slot, reset runway for re-planning
                         flight->slot_order = sim_slots[next_slot_idx].slot_number;
                         flight->runway_assignment = 0;


                    } else {
                        // Cannot move
                        pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  X Flight %s (%s) delayed (%d min) -> CANNOT BE ACCOMMODATED (Runway %d blocked until %d)\n",
                                        flight->plane_id, flight->arr_or_dip == 0 ? "DEP" : "ARR",
                                        flight->delay_minutes, runway_idx + 1, start_time);
                        could_not_move_count++;

                         // Mark flight as unaccommodated
                         flight->slot_order = -2;
                         flight->runway_assignment = -2;
                         // Do NOT decrement sim_slots counts here - it was scheduled but failed
                    }
                    if (pos >= sizeof(temp_buffer) - 200) goto end_sim_loop;
                }
            }
        } // End loop through flights in slot
    } // End loop through slots

end_sim_loop:

    // 4. Update the main AppData records and slots with the simulation results
    memcpy(records, sim_records, count * sizeof(AirlineRecord));
    memcpy(slots, sim_slots, num_slots * sizeof(SlotStatus));

    // 5. Re-run runway planner based on the final slot assignments
    runwayPlanner(records, count, slots, num_slots);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n--- Runway plan updated after simulation ---\n");
     if (pos >= sizeof(temp_buffer) - 200) goto end_sim;

    // Simulation Summary
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "\n--- Simulation Summary ---\n");
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Flights Rerouted (Severe Delay >30min): %d\n", rerouted_count);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Flights Moved to Next Slot: %d\n", moved_to_next_slot_count);
    pos += snprintf(temp_buffer + pos, sizeof(temp_buffer) - pos, "  Flights Unaccommodated (No Space/Time): %d\n", could_not_move_count);
    if (pos >= sizeof(temp_buffer) - 1) goto end_sim;

    // Free allocated memory
    free(sim_records);
    free(sim_slots);

end_sim:
    if (pos >= sizeof(temp_buffer) - 1) {
       const char *trunc_msg = "... [Simulation Log Truncated]\n";
        int space_left = sizeof(temp_buffer) - pos - 1;
        int needed = strlen(trunc_msg) + 1;
        if (space_left < needed) pos -= (needed - space_left);
        strcpy(temp_buffer + pos, trunc_msg);
        pos += strlen(trunc_msg);
    }
    temp_buffer[pos] = '\0';
    appendToOutput(output, max_len, temp_buffer, NULL);
}


// Save Flight and Slot Details to File
void saveFlightAndSlotDetailsToFile(AirlineRecord records[], int count, SlotStatus slots[], int num_slots, char output[], int max_len) {
    FILE *file = fopen("flight_and_slot_report.txt", "w");
    if (!file) {
        char temp_buffer[256];
        snprintf(temp_buffer, sizeof(temp_buffer), "ERROR: Failed to open file 'flight_and_slot_report.txt' for writing.\n");
        appendToOutput(output, max_len, temp_buffer, NULL);
        perror("File open error");
        return;
    }

    time_t now = time(NULL);
    char time_str[100];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(file, "========================================================\n");
    fprintf(file, "       Airline Flight and Slot Allocation Report        \n");
    fprintf(file, "             Generated on: %s             \n", time_str);
    fprintf(file, "========================================================\n\n");

    // --- Flight Details Section ---
    fprintf(file, "===== Final Flight Status =====\n\n");
    fprintf(file, "%-4s %-18s %-14s %-10s %-16s %-16s %-16s %-5s %-9s %-6s %-6s %s\n",
            "ID", "Airline", "Domain", "Type", "From", "To", "Via", "Slot", "Emergency", "Runway", "Delay", "Status");
    fprintf(file, "---- ------------------ -------------- ---------- ---------------- ---------------- ---------------- ----- --------- ------ ------ ------\n");

    int ok_count = 0, unallocated_count = 0, rerouted_count = 0, unaccom_count = 0;

    // Sort records by final slot for reporting consistency
    qsort(records, count, sizeof(AirlineRecord), compareBySlot);

    for (int i = 0; i < count; i++) {
         char slot_str[6];
         char runway_str[7] = "-"; // Default runway to '-'
         const char* status_note = "OK";

         if (records[i].slot_order == 0) { snprintf(slot_str, sizeof(slot_str), "N/A"); unallocated_count++; status_note="Unalloc";}
         else if (records[i].slot_order == -1) { snprintf(slot_str, sizeof(slot_str), "N/A"); rerouted_count++; status_note="Rerouted";}
         else if (records[i].slot_order == -2) { snprintf(slot_str, sizeof(slot_str), "N/A"); unaccom_count++; status_note="Unaccom.";}
         else { // Positively assigned slot
             snprintf(slot_str, sizeof(slot_str), "%d", records[i].slot_order);
             ok_count++;
             // Only show runway if assigned to a valid slot
             if (records[i].runway_assignment > 0) {
                 snprintf(runway_str, sizeof(runway_str), "%d", records[i].runway_assignment);
             }
         }

        fprintf(file, "%-4d %-18.18s %-14.14s %-10s %-16.16s %-16.16s %-16.16s %-5s %-9s %-6s %-6d %s\n",
                i + 1,
                records[i].airline,
                records[i].plane_domain,
                records[i].arr_or_dip == 0 ? "Departure" : "Arrival",
                records[i].starting_destination,
                records[i].end_destination,
                records[i].central_destination[0] != '\0' ? records[i].central_destination : "-",
                slot_str,
                records[i].emergency ? "YES" : "No",
                runway_str,
                records[i].delay_minutes,
                status_note);
    }
     fprintf(file, "---- ------------------ -------------- ---------- ---------------- ---------------- ---------------- ----- --------- ------ ------ ------\n");
     fprintf(file, "Summary: %d OK, %d Unallocated, %d Rerouted, %d Unaccommodated.\n\n", ok_count, unallocated_count, rerouted_count, unaccom_count);


    // --- Slot Allocation Details Section ---
    fprintf(file, "\n===== Slot Allocation Summary =====\n\n");
    fprintf(file, "%-5s %-10s %-10s %-9s %-10s | %-7s %-7s %-7s\n",
            "Slot", "Total", "Int'l", "Domestic", "Capacity", "Runway 1", "Runway 2", "Runway 3");
    fprintf(file, "----- ---------- ---------- --------- ---------- | ------- ------- -------\n");

    for (int i = 0; i < num_slots; i++) {
         char capacity_str[16];
         snprintf(capacity_str, sizeof(capacity_str), "%d / %d", slots[i].planes_allocated, slots[i].max_capacity);
        fprintf(file, "%-5d %-10d %-10d %-9d %-10s | %-7d %-7d %-7d\n",
                slots[i].slot_number,
                slots[i].planes_allocated,
                slots[i].international_count,
                slots[i].domestic_count,
                capacity_str,
                slots[i].runway_assignments[0],
                slots[i].runway_assignments[1],
                slots[i].runway_assignments[2]);
    }
     fprintf(file, "----- ---------- ---------- --------- ---------- | ------- ------- -------\n\n");


    // --- Flights by Slot with Runway Assignments ---
    fprintf(file, "\n===== Flights by Slot and Runway (Final) =====\n");
    for (int slot = 1; slot <= num_slots; slot++) {
        fprintf(file, "\n--- Slot %d ---\n", slot);
        fprintf(file, "%-4s %-12s %-18s %-10s %-6s %-9s %-6s\n",
                 "ID", "Plane ID", "Airline", "Type", "Runway", "Emergency", "Delay");
        fprintf(file, "---- ------------ ------------------ ---------- ------ --------- ------\n");

        bool flights_in_slot = false;
        // Iterate through sorted records
        for (int i = 0; i < count; i++) {
            if (records[i].slot_order == slot) { // Only print flights positively assigned to this slot
                 flights_in_slot = true;
                 char runway_str[7] = "-";
                 if (records[i].runway_assignment > 0) {
                     snprintf(runway_str, sizeof(runway_str), "%d", records[i].runway_assignment);
                 }

                fprintf(file, "%-4d %-12.12s %-18.18s %-10s %-6s %-9s %-6d\n",
                        i + 1, // Use original index + 1 as ID for consistency with Details view
                        records[i].plane_id,
                        records[i].airline,
                        records[i].arr_or_dip == 0 ? "Departure" : "Arrival",
                        runway_str,
                        records[i].emergency ? "YES" : "No",
                        records[i].delay_minutes);
            }
        }
        if (!flights_in_slot) {
             fprintf(file, "(No flights assigned to this slot)\n");
        }
    }

    fprintf(file, "\n====================== End of Report ======================\n");
    fclose(file);

    char temp_buffer[256];
    snprintf(temp_buffer, sizeof(temp_buffer), "Report successfully saved to 'flight_and_slot_report.txt'.\n");
    appendToOutput(output, max_len, temp_buffer, NULL);
}


// Draw wrapped text with basic formatting
void drawWrappedText(const char* text, int x, int y, int max_width, int font_size, Color color, float scroll_offset, int max_height) {
    Font font = GetFontDefault();
    float spacing = 1.0f;
    int current_y_draw = y + PADDING + (int)scroll_offset; // Use separate drawing y
    const char* ptr = text;
    char line_buffer[2048] = {0};
    int line_pos = 0;
    float current_line_width = 0.0f;

    BeginScissorMode(x, y, max_width, max_height);

    while (*ptr) {
         if (strncmp(ptr, "---", 3) == 0 || strncmp(ptr, "===", 3) == 0) {
             if (line_pos > 0) {
                 line_buffer[line_pos] = '\0';
                 if (current_y_draw >= y && current_y_draw + font_size <= y + max_height) {
                     DrawTextEx(font, line_buffer, (Vector2){(float)x + PADDING, (float)current_y_draw}, font_size, spacing, color);
                 }
                 current_y_draw += font_size + 5;
                 line_pos = 0;
                 current_line_width = 0.0f;
             }

             char header_buffer[256] = {0};
             const char* header_start = ptr;
             while (*ptr != '\n' && *ptr != '\0') ptr++;
             int header_len = ptr - header_start;
             if (header_len > sizeof(header_buffer) - 1) header_len = sizeof(header_buffer) - 1;
             strncpy(header_buffer, header_start, header_len);
             header_buffer[header_len] = '\0';

             if (current_y_draw >= y - (font_size + 10) && current_y_draw <= y + max_height) {
                 DrawLine(x + PADDING, current_y_draw - 2, x + max_width - PADDING, current_y_draw - 2, COLOR_BORDER);
                 DrawTextEx(font, header_buffer, (Vector2){(float)x + PADDING, (float)current_y_draw}, font_size + 1, spacing, COLOR_TEXT_DARK);
                 current_y_draw += font_size + 8;
                 DrawLine(x + PADDING, current_y_draw - 2, x + max_width - PADDING, current_y_draw - 2, COLOR_BORDER);
             } else {
                  current_y_draw += font_size + 8;
             }

             if (*ptr == '\n') ptr++;
             continue;
         }

        if (*ptr == '\n') {
            line_buffer[line_pos] = '\0';
             if (current_y_draw >= y && current_y_draw + font_size <= y + max_height) {
                 DrawTextEx(font, line_buffer, (Vector2){(float)x + PADDING, (float)current_y_draw}, font_size, spacing, color);
             }
            current_y_draw += font_size + 5;
            line_pos = 0;
            current_line_width = 0.0f;
            ptr++;
            continue;
        }

        char temp[2] = {*ptr, '\0'};
        Vector2 char_size = MeasureTextEx(font, temp, font_size, spacing);

        if (line_pos > 0 && (current_line_width + char_size.x > max_width - PADDING * 2)) {
            int wrap_pos = line_pos;
            while (wrap_pos > 0 && line_buffer[wrap_pos - 1] != ' ') wrap_pos--;
            if (wrap_pos == 0) wrap_pos = line_pos;

            char save_char = line_buffer[wrap_pos > 0 ? wrap_pos - 1 : wrap_pos];
            if (wrap_pos > 0) line_buffer[wrap_pos - 1] = '\0';
            else line_buffer[wrap_pos] = '\0';

            if (current_y_draw >= y && current_y_draw + font_size <= y + max_height) {
                DrawTextEx(font, line_buffer, (Vector2){(float)x + PADDING, (float)current_y_draw}, font_size, spacing, color);
            }
            current_y_draw += font_size + 5;

            if (wrap_pos > 0) {
                line_buffer[wrap_pos - 1] = save_char;
                int remaining_len = line_pos - wrap_pos;
                memmove(line_buffer, line_buffer + wrap_pos, remaining_len);
                line_pos = remaining_len;
                line_buffer[line_pos] = '\0';
            } else {
                 line_pos = 0;
                 line_buffer[0] = '\0';
            }
            current_line_width = MeasureTextEx(font, line_buffer, font_size, spacing).x;
        }

         if (line_pos < sizeof(line_buffer) - 1) {
             line_buffer[line_pos++] = *ptr;
             current_line_width += char_size.x;
             ptr++;
         } else {
              line_buffer[line_pos] = '\0';
              if (current_y_draw >= y && current_y_draw + font_size <= y + max_height) {
                  DrawTextEx(font, line_buffer, (Vector2){(float)x + PADDING, (float)current_y_draw}, font_size, spacing, color);
              }
              current_y_draw += font_size + 5;
              line_pos = 0;
              current_line_width = 0.0f;
              // Let loop re-evaluate the same char for the new line
         }
    }

    if (line_pos > 0) {
        line_buffer[line_pos] = '\0';
        if (current_y_draw >= y && current_y_draw + font_size <= y + max_height) {
             DrawTextEx(font, line_buffer, (Vector2){(float)x + PADDING, (float)current_y_draw}, font_size, spacing, color);
        }
    }
    EndScissorMode();
}

// Calculate total height of wrapped text (approximation)
int calculateWrappedTextHeight(const char* text, int max_width, int font_size) {
    Font font = GetFontDefault();
    float spacing = 1.0f;
    int current_y = 0;
    const char* ptr = text;
    char line_buffer[2048] = {0}; // Conceptual buffer for wrap check
    int line_pos = 0;
    float current_line_width = 0.0f;

    if (!text || max_width <= 0) return PADDING * 2; // Basic height if no text or no width
    current_y += PADDING; // Top padding

    while (*ptr) {
        if (strncmp(ptr, "---", 3) == 0 || strncmp(ptr, "===", 3) == 0) {
            if (line_pos > 0) {
                 current_y += font_size + 5;
                 line_pos = 0;
                 current_line_width = 0.0f;
            }
             current_y += font_size + 8; // Header spacing
             while (*ptr != '\n' && *ptr != '\0') ptr++;
             if (*ptr == '\n') ptr++;
             continue;
         }

        if (*ptr == '\n') {
            current_y += font_size + 5;
            line_pos = 0;
            current_line_width = 0.0f;
            ptr++;
            continue;
        }

        char temp[2] = {*ptr, '\0'};
        Vector2 char_size = MeasureTextEx(font, temp, font_size, spacing);

        if (line_pos > 0 && (current_line_width + char_size.x > max_width - PADDING * 2)) {
             current_y += font_size + 5;
             line_pos = 0;
             current_line_width = 0.0f;
             // Fall through to add char to new line measurement
        }

        if (line_pos < sizeof(line_buffer) - 1) {
             line_buffer[line_pos++] = *ptr; // Not actually needed, just for line_pos count
             current_line_width += char_size.x;
             ptr++;
        } else {
             current_y += font_size + 5; // Force break
             line_pos = 0;
             current_line_width = 0.0f;
              // Let loop re-process ptr
        }
    }

    if (line_pos > 0) {
        current_y += font_size + 5;
    }
    return current_y + PADDING; // Bottom padding
}

// Draw a single table cell - REFINED
void drawTableCell(const char* text, int x, int y, int width, int height, int font_size, Color text_color, bool is_header, bool is_hovered, bool is_emergency) {
    Font font = GetFontDefault();
    float spacing = 1.0f;
    int text_padding = 8;

    Color bg_color = COLOR_PANEL_BG;
    if (is_header) {
        bg_color = COLOR_HEADER_BG;
    } else if (is_emergency) {
        bg_color = COLOR_EMERGENCY_BG;
        text_color = COLOR_EMERGENCY_TEXT;
    } else if (is_hovered) {
        bg_color = COLOR_BUTTON_HOVER;
    }

    DrawRectangle(x, y, width, height, bg_color);

    DrawLine(x + width - 1, y, x + width - 1, y + height, COLOR_BORDER);
    DrawLine(x, y + height - 1, x + width, y + height - 1, COLOR_BORDER);

    int text_x = x + text_padding;
    int text_y = y + (height - font_size) / 2;

    if (text && strlen(text) > 0) { // Check text != NULL and not empty
        Vector2 text_size = MeasureTextEx(font, text, font_size, spacing);
        if (text_size.x > width - text_padding * 2) {
            char truncated[MAX_STR_LEN];
            strncpy(truncated, text, sizeof(truncated) - 1);
            truncated[sizeof(truncated) - 1] = '\0';
            int current_len = strlen(truncated);
            bool add_ellipsis = false;

            while (current_len > 0 && MeasureTextEx(font, truncated, font_size, spacing).x > width - text_padding * 2) {
                current_len--;
                truncated[current_len] = '\0';
                add_ellipsis = true;
            }
            if (add_ellipsis && current_len > 3) {
                // Check if there's enough space for "...". If not, don't add it or truncate further.
                 if (MeasureTextEx(font, "...", font_size, spacing).x < (width - text_padding*2 - MeasureTextEx(font, truncated, font_size, spacing).x) ) {
                     strcat(truncated, "...");
                 }
                 // Optional: Add more aggressive truncation if ellipsis still doesn't fit.
            }
            DrawTextEx(font, truncated, (Vector2){(float)text_x, (float)text_y}, font_size, spacing, text_color);
        } else {
            DrawTextEx(font, text, (Vector2){(float)text_x, (float)text_y}, font_size, spacing, text_color);
        }
    }
}

// --- New/Modified UI Drawing Helpers ---

// Draw Panel with Border
void DrawPanel(Rectangle bounds, Color color) {
    DrawRectangleRec(bounds, color);
    DrawRectangleLinesEx(bounds, 1.0f, COLOR_BORDER);
}

// Draw Button with Hover/Selected States - CORRECTED
void DrawButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor, Color textColor, bool hovered, bool selected, Color selectedColor) {
    Color currentBg = selected ? selectedColor : (hovered ? hoverColor : baseColor);
    // Use accent color for border if selected
    Color borderColor = selected ? COLOR_ACCENT : COLOR_BORDER;

    DrawRectangleRounded(bounds, ROUNDNESS, SEGMENTS, currentBg);

    // CORRECTED Call: Removed the 5th argument (line thickness)
    DrawRectangleRoundedLines(bounds, ROUNDNESS, SEGMENTS, borderColor);


    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, BASE_FONT_SIZE, 1.0f);
    float textX = bounds.x + (bounds.width - textSize.x) / 2;
    float textY = bounds.y + (bounds.height - BASE_FONT_SIZE) / 2; // Use BASE_FONT_SIZE for vertical centering
    DrawText(text, (int)textX, (int)textY, BASE_FONT_SIZE, textColor);
}


// Draw Text Input Field
void DrawTextInput(Rectangle bounds, const char *label, char *text, int maxLength, bool active) {
     if (label) {
         DrawText(label, bounds.x, bounds.y - BASE_FONT_SIZE - 4, BASE_FONT_SIZE, COLOR_TEXT_MEDIUM);
     }

    DrawRectangleRounded(bounds, ROUNDNESS / 2, SEGMENTS, COLOR_INPUT_BG);
    // CORRECTED Call: Removed the 5th argument (line thickness)
    DrawRectangleRoundedLines(bounds, ROUNDNESS / 2, SEGMENTS, active ? COLOR_INPUT_ACTIVE_BORDER : COLOR_BORDER);


    int textOffsetX = PADDING / 2;
    int textOffsetY = (bounds.height - BASE_FONT_SIZE) / 2;

    BeginScissorMode((int)bounds.x + textOffsetX, (int)bounds.y, (int)bounds.width - textOffsetX * 2, (int)bounds.height);
    DrawText(text, (int)bounds.x + textOffsetX, (int)bounds.y + textOffsetY, BASE_FONT_SIZE, COLOR_TEXT_DARK);
    EndScissorMode();


    if (active) {
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, BASE_FONT_SIZE, 1.0f);
        int cursorX = (int)bounds.x + textOffsetX + (int)textSize.x;
        if (cursorX > bounds.x + bounds.width - textOffsetX) {
             cursorX = (int)bounds.x + (int)bounds.width - textOffsetX;
        }
        if (((int)(GetTime() * 2.0f)) % 2 == 0) {
            DrawRectangle(cursorX + 1, (int)bounds.y + textOffsetY - 2 , 2, BASE_FONT_SIZE + 4, COLOR_TEXT_DARK);
        }
    }
}
// Draw Scrollbar - Refined
void DrawScrollbar(Rectangle bounds, float scrollOffset, float contentHeight, float viewHeight, float *targetScrollOffset, bool *isDragging) {
    if (contentHeight <= viewHeight || viewHeight <= 0) return; // No scrollbar needed or invalid viewHeight

    float scrollbarAreaHeight = bounds.height;
    float maxScroll = contentHeight - viewHeight;
    if (maxScroll < 0) maxScroll = 0;


    float thumbHeight = (viewHeight / contentHeight) * scrollbarAreaHeight;
    if (thumbHeight < 20) thumbHeight = 20;
    if (thumbHeight > scrollbarAreaHeight) thumbHeight = scrollbarAreaHeight;


    float clampedScrollOffset = scrollOffset;
    if (clampedScrollOffset > 0) clampedScrollOffset = 0;
    if (clampedScrollOffset < -maxScroll) clampedScrollOffset = -maxScroll;

    float scrollRatio = (maxScroll == 0) ? 0 : (-clampedScrollOffset) / maxScroll;
    float thumbY = bounds.y + scrollRatio * (scrollbarAreaHeight - thumbHeight);


    Rectangle thumbBounds = { bounds.x, thumbY, bounds.width, thumbHeight };

    DrawRectangleRec(bounds, COLOR_SCROLLBAR_BG);

    Vector2 mousePos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePos, bounds);
    static float clickOffsetY = 0;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hovered) {
        if (CheckCollisionPointRec(mousePos, thumbBounds)) {
            *isDragging = true;
            clickOffsetY = mousePos.y - thumbBounds.y;
        } else {
             float trackClickableHeight = scrollbarAreaHeight - thumbHeight;
             float clickRatio = (trackClickableHeight <= 0) ? 0 : (mousePos.y - bounds.y - thumbHeight / 2) / trackClickableHeight;
             if (clickRatio < 0) clickRatio = 0;
             if (clickRatio > 1) clickRatio = 1;
             *targetScrollOffset = -(clickRatio * maxScroll);
        }
    }

    if (*isDragging) {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            *isDragging = false;
        } else {
             float trackDragableHeight = scrollbarAreaHeight - thumbHeight;
             float targetThumbY = mousePos.y - clickOffsetY;
             float dragRatio = (trackDragableHeight <= 0) ? 0 : (targetThumbY - bounds.y) / trackDragableHeight;
             if (dragRatio < 0) dragRatio = 0;
             if (dragRatio > 1) dragRatio = 1;
             *targetScrollOffset = -(dragRatio * maxScroll);
        }
    }


     Color thumbColor = (*isDragging || (hovered && CheckCollisionPointRec(mousePos, thumbBounds))) ? COLOR_SCROLLBAR_HOVER : COLOR_SCROLLBAR_THUMB;
     if (thumbBounds.y < bounds.y) {thumbBounds.height -= (bounds.y - thumbBounds.y); thumbBounds.y = bounds.y;} // Clamp top
     if (thumbBounds.y + thumbBounds.height > bounds.y + bounds.height) thumbBounds.height = bounds.y + bounds.height - thumbBounds.y; // Clamp bottom
     if (thumbBounds.height > 0) DrawRectangleRec(thumbBounds, thumbColor); // Draw only if valid height
}


// --- Main Function (UI Focus) ---
int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Airline Crew & Runway Management System - Professional UI");
    SetTargetFPS(60);

    static AppData appData = {0};
    int currentSection = 0;
    char output[MAX_OUTPUT_SIZE] = {"Welcome! Please initialize crews or load data using the menu.\n"};
    int outputLines = 1;

    float scrollOffset = 0.0f;
    float targetScrollOffset = 0.0f;
    float menuScrollOffset = 0.0f;
    float targetMenuScrollOffset = 0.0f;

    bool showReinitPrompt = false;
    bool showCrewAllocPrompt = false;
    bool isDraggingOutputScrollbar = false;
    bool isDraggingMenuScrollbar = false;

    AppState appState = AUTH_SELECT;
    UserRole userRole = ROLE_NONE;
    char idInput[MAX_INPUT_LENGTH] = {0};
    char passwordInput[MAX_INPUT_LENGTH] = {0};
    bool idActive = false; // Start inactive
    bool passwordActive = false;
    bool loginFailed = false;
    UserRole selectedRole = ROLE_NONE;

    initializeSlots(appData.slots, NUM_SLOTS, MAX_CAPACITY_PER_SLOT);

    // Main loop
    while (!WindowShouldClose()) {
        // --- Input Handling ---
        Vector2 mousePos = GetMousePosition();
        float mouseWheelMove = GetMouseWheelMove();

        // --- Layout Calculations (Dynamic for Resizing) ---
        int currentScreenWidth = GetScreenWidth();
        int currentScreenHeight = GetScreenHeight();
        Rectangle menuPanelRect = { PADDING, PADDING * 2 + BASE_FONT_SIZE, 270, currentScreenHeight - (PADDING * 3 + BASE_FONT_SIZE) };
        Rectangle menuScrollbarRect = { menuPanelRect.x + menuPanelRect.width, menuPanelRect.y, SCROLLBAR_WIDTH, menuPanelRect.height };
        Rectangle outputScrollbarRect = { currentScreenWidth - PADDING - SCROLLBAR_WIDTH, menuPanelRect.y, SCROLLBAR_WIDTH, menuPanelRect.height };
        Rectangle outputPanelRect = { menuScrollbarRect.x + menuScrollbarRect.width + PADDING, menuPanelRect.y, outputScrollbarRect.x - (menuScrollbarRect.x + menuScrollbarRect.width + PADDING) - PADDING, menuPanelRect.height };


        // --- State-Specific Input Handling ---
        if (appState == AUTH_LOGIN) {
            // Define input boxes for interaction this frame
            Rectangle idBox = {(GetScreenWidth() - 350) / 2, 250, 350, BUTTON_HEIGHT};
            Rectangle passwordBox = {(GetScreenWidth() - 350) / 2, 340, 350, BUTTON_HEIGHT};

            // Activation Logic (Handle clicks FIRST)
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePos, idBox)) {
                    idActive = true;
                    passwordActive = false;
                } else if (CheckCollisionPointRec(mousePos, passwordBox)) {
                    idActive = false;
                    passwordActive = true;
                } else {
                    // Clicked outside input boxes - check buttons later
                }
            }

             // Keyboard Input Logic - Process ONLY if a field is active
             if (idActive) {
                 int maxLen = MAX_INPUT_LENGTH - 1;
                 int key = GetCharPressed();
                 while (key > 0) {
                     if ((key >= 32) && (key <= 125) && (strlen(idInput) < maxLen)) {
                         int len = strlen(idInput);
                         idInput[len] = (char)key;
                         idInput[len + 1] = '\0';
                     }
                     key = GetCharPressed();
                 }
                 if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
                     if (strlen(idInput) > 0) { idInput[strlen(idInput) - 1] = '\0'; }
                 }
                 if (IsKeyPressed(KEY_TAB)) { idActive = false; passwordActive = true; } // Switch focus
             } else if (passwordActive) {
                 int maxLen = MAX_INPUT_LENGTH - 1;
                 int key = GetCharPressed();
                 while (key > 0) {
                     if ((key >= 32) && (key <= 125) && (strlen(passwordInput) < maxLen)) {
                         int len = strlen(passwordInput);
                         passwordInput[len] = (char)key;
                         passwordInput[len + 1] = '\0';
                     }
                     key = GetCharPressed();
                 }
                 if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
                     if (strlen(passwordInput) > 0) { passwordInput[strlen(passwordInput) - 1] = '\0'; }
                 }
                 if (IsKeyPressed(KEY_TAB)) { passwordActive = false; idActive = true; } // Switch focus
             }
        } // End AUTH_LOGIN Input Handling

        // --- General Input Handling (Scrolling etc.) ---
        if (mouseWheelMove != 0) {
             if (CheckCollisionPointRec(mousePos, outputScrollbarRect) || CheckCollisionPointRec(mousePos, outputPanelRect)) {
                 targetScrollOffset += mouseWheelMove * 60.0f;
             } else if (CheckCollisionPointRec(mousePos, menuScrollbarRect) || CheckCollisionPointRec(mousePos, menuPanelRect)) {
                 targetMenuScrollOffset += mouseWheelMove * 40.0f;
             }
        }

        // --- Update Smooth Scroll ---
        float frameTime = GetFrameTime();
        float scrollLerpFactor = 1.0f - expf(-10.0f * frameTime);
        scrollOffset = Lerp(scrollOffset, targetScrollOffset, scrollLerpFactor);
        menuScrollOffset = Lerp(menuScrollOffset, targetMenuScrollOffset, scrollLerpFactor);

        // --- Content Height Calculation ---
        int outputContentHeight = 0;
        int menuContentHeight = 0;
        const char *adminButtonLabels[] = { "Initialize Crews", "Load Flight Data", "Crew Status", "Allocate Slots", "Plan Runways", "View Runway Plan", "Process Flights", "Flight Details", "Assign Delays", "Live Simulation", "Save Report", "Logout" };
        const char *userButtonLabels[] = { "View Runway Plan", "Flight Details", "Live Simulation", "Logout" };
        const char **currentButtonLabels = (userRole == ROLE_ADMIN) ? adminButtonLabels : userButtonLabels;
        int numButtons = (userRole == ROLE_ADMIN) ? (sizeof(adminButtonLabels) / sizeof(adminButtonLabels[0])) : (sizeof(userButtonLabels) / sizeof(userButtonLabels[0]));

        menuContentHeight = numButtons * (BUTTON_HEIGHT + PADDING) + PADDING;

        int viewLogicIndex = currentSection;
        if (userRole == ROLE_USER) {
             if (currentSection == 0) viewLogicIndex = 5; else if (currentSection == 1) viewLogicIndex = 7;
             else if (currentSection == 2) viewLogicIndex = 9; else if (currentSection == 3) viewLogicIndex = 11;
             else viewLogicIndex = -1;
         }

        if (appState == MAIN_MENU) {
            switch (viewLogicIndex) {
                case 5: // Runway Plan view
                     outputContentHeight = 0;
                     for (int slot_idx = 0; slot_idx < NUM_SLOTS; slot_idx++) {
                          outputContentHeight += 40 + 30;
                           int max_flights_in_runway = 0;
                            for(int r=0; r<NUM_RUNWAYS; ++r) if (appData.slots[slot_idx].runway_assignments[r] > max_flights_in_runway) max_flights_in_runway = appData.slots[slot_idx].runway_assignments[r];
                           if (max_flights_in_runway == 0) max_flights_in_runway = 1;
                           if (max_flights_in_runway > MAX_PER_RUNWAY * 2) max_flights_in_runway = MAX_PER_RUNWAY * 2;
                           outputContentHeight += max_flights_in_runway * 30;
                           outputContentHeight += 15;
                     }
                     outputContentHeight += PADDING * 2;
                     break;
                case 7: outputContentHeight = (appData.recordCount + 1) * 35 + PADDING * 2; break; // Flight Details Table
                case 3: outputContentHeight = (NUM_SLOTS + 1) * 35 + PADDING * 2; break; // Slot Status Table
                default: // Wrapped Text Output
                     float calcWidth = outputPanelRect.width > 0 ? outputPanelRect.width : 1.0f;
                     outputContentHeight = calculateWrappedTextHeight(output, (int)calcWidth, BASE_FONT_SIZE - 2);
                     break;
            }
        }

        // --- Clamp Scroll Offsets ---
        float maxOutputScroll = outputContentHeight > outputPanelRect.height ? outputContentHeight - outputPanelRect.height : 0;
        float maxMenuScroll = menuContentHeight > menuPanelRect.height ? menuContentHeight - menuPanelRect.height : 0;

        if (targetScrollOffset > 0) targetScrollOffset = 0;
        if (targetScrollOffset < -maxOutputScroll) targetScrollOffset = -maxOutputScroll;
        if (targetMenuScrollOffset > 0) targetMenuScrollOffset = 0;
        if (targetMenuScrollOffset < -maxMenuScroll) targetMenuScrollOffset = -maxMenuScroll;
        if (scrollOffset > 0) scrollOffset = 0;
        if (scrollOffset < -maxOutputScroll) scrollOffset = -maxOutputScroll;
        if (menuScrollOffset > 0) menuScrollOffset = 0;
        if (menuScrollOffset < -maxMenuScroll) menuScrollOffset = -maxMenuScroll;

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(COLOR_BACKGROUND);

        // Draw Header Bar
        DrawRectangle(0, 0, currentScreenWidth, PADDING * 2 + BASE_FONT_SIZE, COLOR_PANEL_BG);
        DrawLine(0, PADDING * 2 + BASE_FONT_SIZE, currentScreenWidth, PADDING * 2 + BASE_FONT_SIZE, COLOR_BORDER);
        DrawText("Airline Crew & Runway Management System", PADDING, PADDING, BASE_FONT_SIZE, COLOR_TEXT_DARK);

        // State-Specific Drawing
        if (appState == AUTH_SELECT) {
            // --- Admin/User Selection Screen ---
            int boxWidth = 300; int boxHeight = 250;
            Rectangle centerBox = {(currentScreenWidth - boxWidth) / 2, (currentScreenHeight - boxHeight) / 2, boxWidth, boxHeight};
            DrawPanel(centerBox, COLOR_PANEL_BG);
            const char* selectTitle = "Select Role";
            Vector2 selectTitleSize = MeasureTextEx(GetFontDefault(), selectTitle, BASE_FONT_SIZE + 4, 1.0f);
            DrawText(selectTitle, centerBox.x + (centerBox.width - selectTitleSize.x) / 2, centerBox.y + PADDING * 2, BASE_FONT_SIZE + 4, COLOR_TEXT_DARK);
            Rectangle adminBtn = {centerBox.x + PADDING * 3, centerBox.y + PADDING * 7, boxWidth - PADDING * 6, BUTTON_HEIGHT * 1.2f};
            Rectangle userBtn = {centerBox.x + PADDING * 3, adminBtn.y + adminBtn.height + PADDING * 2, boxWidth - PADDING * 6, BUTTON_HEIGHT * 1.2f};
            bool adminHover = CheckCollisionPointRec(mousePos, adminBtn);
            bool userHover = CheckCollisionPointRec(mousePos, userBtn);
            DrawButton(adminBtn, "Administrator", COLOR_ACCENT, COLOR_ACCENT_HOVER, COLOR_TEXT_LIGHT, adminHover, false, BLACK);
            DrawButton(userBtn, "Standard User", COLOR_BUTTON_NORMAL, COLOR_BUTTON_HOVER, COLOR_TEXT_DARK, userHover, false, BLACK);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (adminHover) { appState = AUTH_LOGIN; selectedRole = ROLE_ADMIN; idInput[0] = '\0'; passwordInput[0] = '\0'; loginFailed = false; idActive = true; passwordActive = false; }
                else if (userHover) { appState = AUTH_LOGIN; selectedRole = ROLE_USER; idInput[0] = '\0'; passwordInput[0] = '\0'; loginFailed = false; idActive = true; passwordActive = false; }
            }
        } else if (appState == AUTH_LOGIN) {
            // --- Login Screen Drawing ---
             int boxWidth = 400; int boxHeight = 450;
             Rectangle centerBox = {(currentScreenWidth - boxWidth) / 2, (currentScreenHeight - boxHeight) / 2, boxWidth, boxHeight};
             DrawPanel(centerBox, COLOR_PANEL_BG);
             const char* loginTitle = TextFormat("%s Login", selectedRole == ROLE_ADMIN ? "Administrator" : "User");
             Vector2 loginTitleSize = MeasureTextEx(GetFontDefault(), loginTitle, BASE_FONT_SIZE + 4, 1.0f);
             DrawText(loginTitle, centerBox.x + (centerBox.width - loginTitleSize.x) / 2, centerBox.y + PADDING * 2, BASE_FONT_SIZE + 4, COLOR_TEXT_DARK);

             Rectangle idBox = {centerBox.x + PADDING * 2, centerBox.y + PADDING * 8, boxWidth - PADDING * 4, BUTTON_HEIGHT};
             Rectangle passwordBox = {centerBox.x + PADDING * 2, idBox.y + idBox.height + PADDING * 4, boxWidth - PADDING * 4, BUTTON_HEIGHT};
             DrawTextInput(idBox, "User ID", idInput, MAX_INPUT_LENGTH, idActive);
             char maskedPassword[MAX_INPUT_LENGTH] = {0};
             memset(maskedPassword, '*', strlen(passwordInput));
             DrawTextInput(passwordBox, "Password", maskedPassword, MAX_INPUT_LENGTH, passwordActive);

             Rectangle loginBtn = {centerBox.x + PADDING * 2, passwordBox.y + passwordBox.height + PADDING * 3, (boxWidth - PADDING * 5) / 2, BUTTON_HEIGHT};
             Rectangle backBtn = {loginBtn.x + loginBtn.width + PADDING, loginBtn.y, (boxWidth - PADDING * 5) / 2, BUTTON_HEIGHT};
             bool loginHover = CheckCollisionPointRec(mousePos, loginBtn);
             bool backHover = CheckCollisionPointRec(mousePos, backBtn);
             DrawButton(loginBtn, "Login", COLOR_ACCENT, COLOR_ACCENT_HOVER, COLOR_TEXT_LIGHT, loginHover, false, BLACK);
             DrawButton(backBtn, "Back", COLOR_BUTTON_NORMAL, COLOR_BUTTON_HOVER, COLOR_TEXT_DARK, backHover, false, BLACK);

             // Login Logic Trigger (moved here for drawing context)
             bool attemptLogin = false;
             if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && loginHover) attemptLogin = true;
             if (IsKeyPressed(KEY_ENTER) && (idActive || passwordActive || loginHover)) attemptLogin = true;

             if (attemptLogin) {
                 bool success = false;
                  if (selectedRole == ROLE_ADMIN && strcmp(idInput, "admin") == 0 && strcmp(passwordInput, "adminpass") == 0) success = true;
                  else if (selectedRole == ROLE_USER && strcmp(idInput, "user") == 0 && strcmp(passwordInput, "userpass") == 0) success = true;

                 if (success) {
                      userRole = selectedRole; appState = MAIN_MENU; currentSection = 0;
                      snprintf(output, MAX_OUTPUT_SIZE, "Login successful. Welcome %s!\n", (userRole == ROLE_ADMIN) ? "Admin" : "User");
                      scrollOffset = targetScrollOffset = 0; menuScrollOffset = targetMenuScrollOffset = 0;
                      loginFailed = false; idInput[0] = '\0'; passwordInput[0] = '\0'; idActive = false; passwordActive = false;
                 } else {
                     loginFailed = true; passwordInput[0] = '\0'; idActive = true; passwordActive = false;
                 }
             }

            // Back Button Logic
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && backHover) {
                appState = AUTH_SELECT; idInput[0] = '\0'; passwordInput[0] = '\0'; loginFailed = false; idActive = false; passwordActive = false;
            }

            // Login Failure Message
             if (loginFailed) {
                 const char* errorMsg = "Invalid ID or Password.";
                 Vector2 errorSize = MeasureTextEx(GetFontDefault(), errorMsg, BASE_FONT_SIZE - 2, 1.0f);
                 DrawText(errorMsg, centerBox.x + (centerBox.width - errorSize.x) / 2, loginBtn.y + loginBtn.height + PADDING * 2, BASE_FONT_SIZE - 2, COLOR_NO);
             }
        } else if (appState == MAIN_MENU) {
            // --- Main Application Screen ---
            DrawPanel(menuPanelRect, COLOR_MENU_BG);
            DrawPanel(outputPanelRect, COLOR_PANEL_BG);

            // --- Draw Menu Buttons ---
            BeginScissorMode((int)menuPanelRect.x, (int)menuPanelRect.y, (int)menuPanelRect.width, (int)menuPanelRect.height);
            int buttonY = (int)menuPanelRect.y + PADDING + (int)menuScrollOffset;
            for (int i = 0; i < numButtons; i++) {
                Rectangle button = {(float)menuPanelRect.x + PADDING, (float)buttonY, (float)menuPanelRect.width - PADDING * 2, (float)BUTTON_HEIGHT};
                 if (button.y + button.height > menuPanelRect.y && button.y < menuPanelRect.y + menuPanelRect.height) {
                     bool isSelected = (currentSection == i);
                     bool isHovered = CheckCollisionPointRec(mousePos, button);
                     DrawButton(button, currentButtonLabels[i],
                                isSelected ? COLOR_ACCENT : COLOR_BUTTON_NORMAL,
                                isSelected ? COLOR_ACCENT_HOVER : COLOR_BUTTON_HOVER,
                                isSelected ? COLOR_TEXT_LIGHT : COLOR_TEXT_DARK,
                                isHovered, isSelected, COLOR_ACCENT);

                     // --- Button Click Logic (Simplified) ---
                     if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isHovered) {
                          currentSection = i; // Set the new section
                          targetScrollOffset = scrollOffset = 0; // Reset output scroll
                          output[0] = '\0'; // Clear output buffer
                          outputLines = 0; // Reset line count

                          // Determine the action logic based on role and button index 'i'
                          int actionLogicIndex = i; // Default for admin
                          if (userRole == ROLE_USER) { // Map user index to admin logic index
                              if (i == 0) actionLogicIndex = 5; else if (i == 1) actionLogicIndex = 7;
                              else if (i == 2) actionLogicIndex = 9; else if (i == 3) actionLogicIndex = 11;
                              else actionLogicIndex = -1; // Invalid index
                          }

                          // Execute action based on actionLogicIndex
                           switch (actionLogicIndex) {
                                case 0: // Init Crews
                                    if (userRole == ROLE_ADMIN) {
                                        if (!appData.initialized) { initializeCrewTeams(appData.domestic, appData.international, appData.backup); appData.initialized = 1; snprintf(output, MAX_OUTPUT_SIZE, "Crews initialized.\n"); }
                                        else { showReinitPrompt = true; }
                                    } break;
                                case 1: // Load Data
                                     if (userRole == ROLE_ADMIN) {
                                         appData.recordCount = readCSV("test2.csv", appData.records, MAX_RECORDS);
                                         if (appData.recordCount >= 0) { appData.dataLoaded = 1; appData.slotsAllocated = 0; snprintf(output, MAX_OUTPUT_SIZE, "Loaded %d records.\nAllocate Slots next.\n", appData.recordCount); }
                                         else { snprintf(output, MAX_OUTPUT_SIZE, "ERROR loading CSV.\n"); }
                                     } break;
                                case 2: // Crew Status
                                     if (userRole == ROLE_ADMIN) {
                                         if (appData.initialized) { printCrewStatus(appData.domestic, appData.international, appData.backup, output, MAX_OUTPUT_SIZE); }
                                         else { snprintf(output, MAX_OUTPUT_SIZE, "Init Crews first.\n"); }
                                     } break;
                                case 3: // Allocate Slots
                                     if (userRole == ROLE_ADMIN) {
                                         if (appData.dataLoaded) { slotAllocator(appData.records, appData.recordCount, appData.slots, NUM_SLOTS); appData.slotsAllocated = 1; snprintf(output, MAX_OUTPUT_SIZE, "Slots allocated. Displaying Status.\n"); }
                                         else { snprintf(output, MAX_OUTPUT_SIZE, "Load Data first.\n"); }
                                     } break;
                                case 4: // Plan Runways
                                     if (userRole == ROLE_ADMIN) {
                                         if (appData.dataLoaded && appData.slotsAllocated) { runwayPlanner(appData.records, appData.recordCount, appData.slots, NUM_SLOTS); snprintf(output, MAX_OUTPUT_SIZE, "Runways planned.\nView Plan next.\n"); }
                                         else { snprintf(output, MAX_OUTPUT_SIZE, "Load Data & Allocate Slots first.\n"); }
                                     } break;
                                case 5: // View Runway Plan
                                     if (appData.dataLoaded && appData.slotsAllocated) { snprintf(output, MAX_OUTPUT_SIZE, "Displaying Runway Plan.\n"); }
                                     else { snprintf(output, MAX_OUTPUT_SIZE, "Load, Allocate, & Plan first.\n"); } break;
                                case 6: // Process Flights (w/Alloc)
                                     if (userRole == ROLE_ADMIN) {
                                         if (appData.initialized && appData.dataLoaded && appData.slotsAllocated) { processFlightsWithSlotAllocation(appData.records, appData.recordCount, appData.domestic, appData.international, appData.backup, output, MAX_OUTPUT_SIZE); }
                                         else { snprintf(output, MAX_OUTPUT_SIZE, "Need Init, Load, Alloc.\n"); }
                                     } break;
                                case 7: // Flight Details
                                     if (appData.dataLoaded) { snprintf(output, MAX_OUTPUT_SIZE, "Displaying Flight Details.\n"); }
                                     else { snprintf(output, MAX_OUTPUT_SIZE, "Load Data first.\n"); } break;
                                case 8: // Assign Delays
                                     if (userRole == ROLE_ADMIN) {
                                         if (appData.dataLoaded && appData.slotsAllocated) { airplaneDelay(appData.records, appData.recordCount, appData.slots, NUM_SLOTS, output, MAX_OUTPUT_SIZE); }
                                         else { snprintf(output, MAX_OUTPUT_SIZE, "Load Data & Allocate Slots first.\n"); }
                                     } break;
                                case 9: // Live Simulation
                                     if (userRole == ROLE_ADMIN) {
                                         if (appData.dataLoaded && appData.slotsAllocated) { simulateSlotWithDelays(appData.records, appData.recordCount, appData.slots, NUM_SLOTS, output, MAX_OUTPUT_SIZE); }
                                         else { snprintf(output, MAX_OUTPUT_SIZE, "Load, Alloc, Assign Delays first.\n"); }
                                     } else { // User view
                                          if (appData.dataLoaded && appData.slotsAllocated) { // Only show if data is likely there
                                              if (strlen(output) == 0) snprintf(output, MAX_OUTPUT_SIZE, "Displaying current status.\nAdmin runs simulation.\n");
                                              // else keep showing previous log
                                          } else {
                                              snprintf(output, MAX_OUTPUT_SIZE, "Data not available for simulation view.\n");
                                          }
                                     } break;
                                case 10: // Save Report
                                     if (userRole == ROLE_ADMIN) {
                                         if (appData.dataLoaded && appData.slotsAllocated) { saveFlightAndSlotDetailsToFile(appData.records, appData.recordCount, appData.slots, NUM_SLOTS, output, MAX_OUTPUT_SIZE); }
                                         else { snprintf(output, MAX_OUTPUT_SIZE, "Load & Allocate first.\n"); }
                                     } break;
                                case 11: // Logout
                                     appState = AUTH_SELECT; userRole = ROLE_NONE;
                                     idInput[0]='\0'; passwordInput[0]='\0';
                                     idActive = false; passwordActive = false; // Deactivate on logout
                                     // Optionally reset appData here
                                     break;
                                default: break; // Should not happen
                           }
                     } // End button click
                 } // End visibility check
                buttonY += BUTTON_HEIGHT + PADDING;
            }
            EndScissorMode();

            DrawScrollbar(menuScrollbarRect, menuScrollOffset, menuContentHeight, menuPanelRect.height, &targetMenuScrollOffset, &isDraggingMenuScrollbar);


            // --- Draw Output Content ---
            // Determine view index again for drawing phase
            int viewLogicIndexDrawPhase = currentSection;
            if (userRole == ROLE_USER) {
                if (currentSection == 0) viewLogicIndexDrawPhase = 5;
                else if (currentSection == 1) viewLogicIndexDrawPhase = 7;
                else if (currentSection == 2) viewLogicIndexDrawPhase = 9; // For simulation view/log
                else if (currentSection == 3) viewLogicIndexDrawPhase = 11;// Logout case handled elsewhere
            }

             switch (viewLogicIndexDrawPhase) {
                 case 3: // Slot Status Table
                     printSlotStatus(appData.slots, NUM_SLOTS, output, &outputLines, &scrollOffset, (int)outputPanelRect.x, (int)outputPanelRect.y, (int)outputPanelRect.width, (int)outputPanelRect.height);
                     break;
                 case 5: // View Runway Plan
                     displayRunwayAllocationPlan(appData.records, appData.recordCount, appData.slots, NUM_SLOTS, output, MAX_OUTPUT_SIZE, (int)outputPanelRect.x, (int)outputPanelRect.y, (int)outputPanelRect.width, (int)outputPanelRect.height, &scrollOffset);
                     break;
                 case 7: // Display Flight Details
                     displayFlightDetails(appData.records, appData.recordCount, output, &outputLines, &scrollOffset, (int)outputPanelRect.x, (int)outputPanelRect.y, (int)outputPanelRect.width, (int)outputPanelRect.height);
                     break;
                 default: // All other sections use wrapped text output
                     drawWrappedText(output, (int)outputPanelRect.x, (int)outputPanelRect.y, (int)outputPanelRect.width, BASE_FONT_SIZE - 2, COLOR_TEXT_DARK, scrollOffset, (int)outputPanelRect.height);
                     break;
             }

            DrawScrollbar(outputScrollbarRect, scrollOffset, outputContentHeight, outputPanelRect.height, &targetScrollOffset, &isDraggingOutputScrollbar);

            // --- Draw Popups ---
            if (showReinitPrompt || showCrewAllocPrompt) {
                 DrawRectangle(0, 0, currentScreenWidth, currentScreenHeight, COLOR_PROMPT_OVERLAY);

                 int promptWidth = 450;
                 int promptHeight = 220;
                 Rectangle promptBox = {(currentScreenWidth - promptWidth) / 2, (currentScreenHeight - promptHeight) / 2, promptWidth, promptHeight};
                 DrawPanel(promptBox, COLOR_PANEL_BG);

                 const char *promptTitle = showReinitPrompt ? "Confirm Reinitialization" : "Manual Allocation";
                 const char *promptLine1 = showReinitPrompt ? "Crew teams have already been initialized." : "Manual Crew Allocation is not yet implemented.";
                 const char *promptLine2 = showReinitPrompt ? "Do you want to discard current status and reinitialize?" : "Proceed with automatic allocation based on current data?";

                  Vector2 titleSizePopup = MeasureTextEx(GetFontDefault(), promptTitle, BASE_FONT_SIZE, 1.0f);
                  DrawText(promptTitle, promptBox.x + (promptBox.width - titleSizePopup.x) / 2, promptBox.y + PADDING, BASE_FONT_SIZE, COLOR_TEXT_DARK);
                  DrawLine((int)promptBox.x, (int)promptBox.y + PADDING * 2 + BASE_FONT_SIZE, (int)promptBox.x + promptWidth, (int)promptBox.y + PADDING * 2 + BASE_FONT_SIZE, COLOR_BORDER);
                  DrawText(promptLine1, (int)promptBox.x + PADDING, (int)promptBox.y + PADDING * 4 + BASE_FONT_SIZE, BASE_FONT_SIZE -2, COLOR_TEXT_MEDIUM);
                  DrawText(promptLine2, (int)promptBox.x + PADDING, (int)promptBox.y + PADDING * 6 + BASE_FONT_SIZE, BASE_FONT_SIZE -2, COLOR_TEXT_MEDIUM);

                 Rectangle yesBtn = {promptBox.x + PADDING * 2, promptBox.y + promptHeight - BUTTON_HEIGHT - PADDING * 2, (promptWidth - PADDING * 5) / 2, BUTTON_HEIGHT};
                 Rectangle noBtn = {yesBtn.x + yesBtn.width + PADDING, yesBtn.y, (promptWidth - PADDING * 5) / 2, BUTTON_HEIGHT};
                 bool yesHover = CheckCollisionPointRec(mousePos, yesBtn);
                 bool noHover = CheckCollisionPointRec(mousePos, noBtn);

                 DrawButton(yesBtn, "Yes", COLOR_YES, ColorBrightness(COLOR_YES, 0.2f), COLOR_TEXT_LIGHT, yesHover, false, BLACK);
                 DrawButton(noBtn, "No", COLOR_NO, ColorBrightness(COLOR_NO, 0.2f), COLOR_TEXT_LIGHT, noHover, false, BLACK);

                 if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                     if (yesHover) {
                         if (showReinitPrompt) {
                             initializeCrewTeams(appData.domestic, appData.international, appData.backup); appData.initialized = 1;
                             snprintf(output, MAX_OUTPUT_SIZE, "Crews reinitialized.\n");
                             showReinitPrompt = false;
                         } else if (showCrewAllocPrompt) {
                              snprintf(output, MAX_OUTPUT_SIZE, "Proceeding with auto alloc (placeholder).\n");
                              showCrewAllocPrompt = false;
                         }
                     } else if (noHover || !CheckCollisionPointRec(mousePos, promptBox)) { // Click No or outside
                         bool wasShowing = showReinitPrompt || showCrewAllocPrompt;
                         showReinitPrompt = false; showCrewAllocPrompt = false;
                         if (wasShowing) appendToOutput(output, MAX_OUTPUT_SIZE, "Operation cancelled.\n", NULL);
                     }
                 }
            } // End popups

        } // End MAIN_MENU state

        EndDrawing();
    } // End main loop

    CloseWindow();
    return 0;
}