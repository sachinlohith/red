typedef struct Queue {
    char *data;
    int front, rear, size, capacity;
}Queue;

Queue* createQueue(int maxElements) {
    Queue *result;
    result = (Queue *) malloc(sizeof(Queue));
    result->data = (char *) malloc(maxElements*sizeof(char));
    result->front = 0;
    result->size = 0;
    result->rear = -1;
    result->capacity = maxElements;
    return result;
}

void add(Queue *queue, char value) {
    if (queue->size == queue->capacity) {
        printf("Queue full\n");
    } else {
        queue->size++;
        queue->rear++;
        if(queue->rear == queue->capacity) {
            queue->rear = 0;
        }
        queue->data[queue->rear] = value;
    }
}

void delete(Queue *queue) {
    if (queue->size == 0) {
        printf("Queue empty\n");
        return;
    } else {
        queue->size--;
        queue->front++;
        if(queue->front == queue->capacity) {
            queue->front = 0;
        }
    }
    return;
}

char front(Queue *queue) {
    if(queue->size == 0) {
        printf("Queue empty\n");
        return '\0';
    }
    return queue->data[queue->front];
}

void display(Queue *queue) {
    if (queue->size == 0) {
        printf("Queue empty\n");
        return;
    }
    if (queue->front > queue->rear) {
        for (int i = queue->front; i < queue->capacity; i++) {
            printf("%c ", queue->data[i]);
        }
        for (int i = 0; i < queue->rear; i++) {
            printf("%c ", queue->data[i]);
        }
        printf("\n");
    } else {
        for (int i = queue->front; i <= queue->rear; i++) {
            printf("%c ", queue->data[i]);
        }
        printf("\n");
    }
}
