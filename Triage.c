#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A simple structure to hold all information for a single patient.
typedef struct {
    char name[60];
    int priority_level; // A lower number means higher priority (e.g., 1 is the most critical).
    int patient_id;
} Patient;

// The Min-Heap structure, which will function as our Priority Queue.
typedef struct {
    Patient* patients; // A pointer to an array of Patient structs.
    int size;          // The current number of patients in the heap.
    int capacity;      // The total allocated capacity of the patient array.
} MinHeap;

// The dynamic array structure for logging treated patients.
typedef struct {
    Patient* patients;
    int size;
    int capacity;
} TreatedLog;

// A main structure to hold pointers to our heap and log.
typedef struct {
    MinHeap* waiting_list;
    TreatedLog* treated_log;
    int next_patient_id;
} TriageSystem;

// Creates and returns a pointer to a new Min-Heap.
MinHeap* create_heap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->patients = (Patient*)malloc(capacity * sizeof(Patient));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// A utility function to swap two Patient structs.
void swap(Patient* a, Patient* b) {
    Patient temp = *a;
    *a = *b;
    *b = temp;
}

// This function restores the heap property by moving a node up the tree.
// It's used after inserting a new patient.
void heapify_up(MinHeap* heap, int index) {
    int parent_index = (index - 1) / 2;
    // Keep swapping up as long as the child is higher priority (smaller number) than its parent.
    if (index > 0 && heap->patients[index].priority_level < heap->patients[parent_index].priority_level) {
        swap(&heap->patients[index], &heap->patients[parent_index]);
        heapify_up(heap, parent_index);
    }
}

// This function restores the heap property by moving a node down the tree.
// It's used after removing the top patient.
void heapify_down(MinHeap* heap, int index) {
    int smallest = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    // Find the smallest among the node and its children.
    if (left_child < heap->size && heap->patients[left_child].priority_level < heap->patients[smallest].priority_level) {
        smallest = left_child;
    }
    if (right_child < heap->size && heap->patients[right_child].priority_level < heap->patients[smallest].priority_level) {
        smallest = right_child;
    }

    // If the smallest is not the current node, swap them and continue heapifying down.
    if (smallest != index) {
        swap(&heap->patients[index], &heap->patients[smallest]);
        heapify_down(heap, smallest);
    }
}

// Inserts a new patient into the heap.
void insert_patient_to_heap(MinHeap* heap, Patient patient) {
    if (heap->size == heap->capacity) {
        printf("Error: Waiting list is full. Cannot add more patients.\n");
        return;
    }
    // Add the new patient to the end and then heapify up to place it correctly.
    heap->size++;
    int index = heap->size - 1;
    heap->patients[index] = patient;
    heapify_up(heap, index);
}

// Extracts the highest-priority patient (the root) from the heap.
Patient extract_min(MinHeap* heap) {
    if (heap->size <= 0) {
        Patient empty_patient = {"", -1, -1}; // Return an invalid patient to indicate error.
        return empty_patient;
    }
    if (heap->size == 1) {
        heap->size--;
        return heap->patients[0];
    }

    // Save the root, move the last element to the root, and heapify down.
    Patient root = heap->patients[0];
    heap->patients[0] = heap->patients[heap->size - 1];
    heap->size--;
    heapify_down(heap, 0);

    return root;
}

// Initializes the entire triage system.
TriageSystem* create_triage_system(int initial_capacity) {
    TriageSystem* system = (TriageSystem*)malloc(sizeof(TriageSystem));
    system->waiting_list = create_heap(initial_capacity);
    system->treated_log = (TreatedLog*)malloc(sizeof(TreatedLog));
    system->treated_log->patients = (Patient*)malloc(initial_capacity * sizeof(Patient));
    system->treated_log->size = 0;
    system->treated_log->capacity = initial_capacity;
    system->next_patient_id = 1;
    return system;
}

// Frees all dynamically allocated memory to prevent memory leaks.
void free_triage_system(TriageSystem* system) {
    free(system->waiting_list->patients);
    free(system->waiting_list);
    free(system->treated_log->patients);
    free(system->treated_log);
    free(system);
}

// Adds a new patient to the waiting list.
void add_patient(TriageSystem* system, const char* name, int priority) {
    Patient new_patient;
    strcpy(new_patient.name, name);
    new_patient.priority_level = priority;
    new_patient.patient_id = system->next_patient_id++;

    insert_patient_to_heap(system->waiting_list, new_patient);
    printf("NEW PATIENT: '%s' added to waiting list with priority %d.\n", name, priority);
}

// Treats the next highest-priority patient.
void treat_next_patient(TriageSystem* system) {
    if (system->waiting_list->size == 0) {
        printf("SYSTEM: No patients in the waiting list to treat.\n");
        return;
    }

    Patient patient_to_treat = extract_min(system->waiting_list);

    printf("\nTREATING NEXT PATIENT:\n");
    printf("  ID: %d, Name: %s, Priority: %d\n", patient_to_treat.patient_id, patient_to_treat.name, patient_to_treat.priority_level);

    // Add the treated patient to our dynamic array log.
    if (system->treated_log->size < system->treated_log->capacity) {
        system->treated_log->patients[system->treated_log->size++] = patient_to_treat;
    } else {
        // Here, a real system would reallocate more memory for the log.
        printf("Warning: Treated log is full.\n");
    }
}

// Displays the status of the waiting list.
void view_waiting_list(TriageSystem* system) {
    printf("\n--- Current Waiting List ---\n");
    if (system->waiting_list->size == 0) {
        printf("  (The waiting list is empty)\n");
    } else {
        printf("  Total patients waiting: %d\n", system->waiting_list->size);
        Patient next_patient = system->waiting_list->patients[0]; // The root of the heap is next.
        printf("  Next to be treated: ID: %d, Name: %s, Priority: %d\n", next_patient.patient_id, next_patient.name, next_patient.priority_level);
    }
    printf("--------------------------\n");
}

// Displays the log of all treated patients.
void view_treated_log(TriageSystem* system) {
    printf("\n--- Log of Treated Patients ---\n");
    if (system->treated_log->size == 0) {
        printf("  (No patients have been treated yet)\n");
    } else {
        for (int i = 0; i < system->treated_log->size; i++) {
            Patient p = system->treated_log->patients[i];
            printf("  ID: %d, Name: %s, Priority: %d\n", p.patient_id, p.name, p.priority_level);
        }
    }
    printf("-----------------------------\n");
}

int main() {
    // Create the system with an initial capacity of 20 patients.
    TriageSystem* er = create_triage_system(20);

    printf("--- Emergency Room Simulation Started ---\n\n");
    // Add patients with priorities out of order to show the heap works correctly.
    add_patient(er, "Ravi Kumar (Stable Condition)", 3);
    add_patient(er, "Sita Sharma (Critical Injury)", 1);
    add_patient(er, "Amit Patel (Urgent Care)", 2);
    add_patient(er, "Priya Singh (Minor Issue)", 4);

    view_waiting_list(er);

    // Treat the first two patients. The system should automatically pick Sita (1) then Amit (2).
    treat_next_patient(er);
    treat_next_patient(er);

    view_waiting_list(er);

    printf("\n--- A new, very critical patient arrives ---\n");
    add_patient(er, "John Doe (Head Trauma)", 1);

    view_waiting_list(er);

    // Treat the remaining patients. John (new 1) should be next.
    treat_next_patient(er);
    treat_next_patient(er);
    treat_next_patient(er);

    treat_next_patient(er); // Should show that the list is now empty.

    // Finally, display the complete log of all patients who were treated.
    view_treated_log(er);

    // Clean up all the memory we allocated.
    free_triage_system(er);

    return 0;
}
