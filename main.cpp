#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX_SIZE 256
#define IGCT 8

typedef enum {
    INT,
    TWIN
}oType;

typedef struct sObject {
    oType type;
    unsigned char marked;

    struct sObject* next;

    union {
        int value;
        struct 
        {
            struct sObject* head;
            struct sObject* tail;
            
        };
        
    };
}Object;

typedef struct
{
    Object* stack[STACK_MAX_SIZE];
    int stackSize;

    Object* firstObject;

    int numObjects;
    int maxObjects;

}vm;

void push(vm* vm, Object* value) {
    vm->stack[vm->stackSize++] = value;
}

Object* pop(vm* vm) {
    return vm->stack[--vm->stackSize];
}

vm* newVm() {
    vm* mainVm = (vm*)malloc(sizeof(vm));
    mainVm->stackSize = 0;
    mainVm->firstObject = NULL;
    mainVm->numObjects = 0;
    mainVm->maxObjects = IGCT;
    return mainVm;
}

void mark(Object* object) {
    if(object->marked) return;

    object->marked = 1;

    if(object->type == TWIN){
        mark(object->head);
        mark(object->tail);
    }
}

void markAll(vm* vm) {
    for (int i = 0; i < vm->stackSize; i++) {
        mark(vm->stack[i]);
    }
    
}

void marksweep(vm* vm) {
    Object** object = &vm->firstObject;
    while(*object) {
        if(!(*object)->marked) {
            Object* unreached = *object;
            *object = unreached->next;
            free(unreached);

            vm->numObjects--;
        } else {
            (*object)->marked = 0;
            object = &(*object)->next;
        }
    }
}


void gc(vm* vm) {
    int numObjects = vm->numObjects;
    markAll(vm);
    marksweep(vm);

    vm->maxObjects = vm->numObjects * 2;
    
    printf("Deleted %d objects, %d left. \n", numObjects - vm->numObjects, vm->numObjects);
}

Object* newObject(vm* vm, oType type) {
    if(vm->numObjects == vm->maxObjects) gc(vm);

    Object* object = (Object*) malloc(sizeof(Object));
    object->type = type;
    object->next = vm->firstObject;
    vm->firstObject = object;
    object->marked = 0;

    vm->numObjects++;
    return object;
}

//Custom push any types to virtual machine
void pushInt(vm* vm, int intV) {
    Object* object = newObject(vm, INT);
    object->value = intV;
    push(vm, object);
}

Object* pushTwin(vm* vm) {
    Object* object = newObject(vm, TWIN);
    object->tail = pop(vm);
    object->head = pop(vm);

    push(vm, object);
    return object;
}


void freeVm(vm* vm) {
    vm->stackSize = 0;
    gc(vm);
    free(vm);
}

void printObj(Object* object) {
    switch (object->type)
    {
    case INT:
        printf("%d", object->value);
        break;
    case TWIN:
        printf("(");
        printObj(object->head);
        printf(", ");
        printObj(object->tail);
        printf(")");
        break;
    
    default:
        break;
    }
}

void first_test() {
	printf("1: Objects on the stack are preserved.\n");
	vm* mainVm = newVm();
	pushInt(mainVm, 1);
	pushInt(mainVm, 2);

	gc(mainVm);
	freeVm(mainVm);
}

void second_test() {
    printf("2: Unreached objects are collected.\n");
    vm* mainVm = newVm();
    pushInt(mainVm, 1);
    pushInt(mainVm, 2);
    pop(mainVm);
    pop(mainVm);

    gc(mainVm);
    freeVm(mainVm);
}

void third_test() {
    printf("3: Reach the nested objects.\n");
    vm* mainVm = newVm();
    pushInt(mainVm, 1);
    pushInt(mainVm, 2);
    pushTwin(mainVm);
    pushInt(mainVm, 3);
    pushInt(mainVm, 4);
    pushTwin(mainVm);
    pushTwin(mainVm);

    gc(mainVm);
    freeVm(mainVm);
}

void fouth_test() {
    printf("4: Cycles. \n");
    vm* mainVm = newVm();
    pushInt(mainVm, 1);
    pushInt(mainVm, 2);
    Object* a = pushTwin(mainVm);
    pushInt(mainVm, 3);
    pushInt(mainVm, 4);
    Object* b = pushTwin(mainVm);

    a->tail = b;
    b->tail = a;

    gc(mainVm);
    freeVm(mainVm);
}

void performance() {
    printf("Performance of GC.\n");
    vm* maiVm = newVm();

    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 20; j++) {
            pushInt(maiVm, i);
        }

        for(int k = 0; k < 20; k++) {
            pop(maiVm);
        }
        
    }
    freeVm(maiVm);
    
}

int main(int argc, const char** argv) {
    first_test();
    second_test();
    third_test();
    fouth_test();
    performance();

    return(0);
}