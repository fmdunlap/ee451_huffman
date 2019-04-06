// C program for Huffman Coding 
#include "../util.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>
#include <string.h>

// This constant can be avoided by explicitly 
// calculating height of Huffman Tree 
#define MAX_TREE_HT 100 

// I miss STL... :(
int ZERO_SENT = 0;
int ONE_SENT = 1;

// A Huffman tree node 
struct SerialNode{
    unsigned char data;
    unsigned freq;
    bool null;
};

struct MinHeapNode { 

	// One of the input characters 
	unsigned char data; 

	// Frequency of the character 
	unsigned freq; 

	// Left and right child of this node 
	struct MinHeapNode *left, *right; 
}; 

// A Min Heap: Collection of 
// min-heap (or Huffman tree) nodes 
struct MinHeap { 

	// Current size of min heap 
	unsigned size; 

	// capacity of min heap 
	unsigned capacity; 

	// Array of minheap node pointers 
	struct MinHeapNode** array; 
}; 

// A utility function allocate a new 
// min heap node with given character 
// and frequency of the character 
struct MinHeapNode* newNode(char data, unsigned freq) 
{ 
	struct MinHeapNode* temp 
		= (struct MinHeapNode*) malloc(sizeof(struct MinHeapNode)); 

	temp->left = temp->right = NULL; 
	temp->data = data; 
	temp->freq = freq; 

	return temp; 
} 

// A utility function to create 
// a min heap of given capacity 
struct MinHeap* createMinHeap(unsigned capacity) 

{ 

	struct MinHeap* minHeap 
		= (struct MinHeap*)malloc(sizeof(struct MinHeap)); 

	// current size is 0 
	minHeap->size = 0; 

	minHeap->capacity = capacity; 

	minHeap->array 
		= (struct MinHeapNode**)malloc(minHeap-> 
capacity * sizeof(struct MinHeapNode*)); 
	return minHeap; 
} 

// A utility function to 
// swap two min heap nodes 
void swapMinHeapNode(struct MinHeapNode** a, 
					struct MinHeapNode** b) 

{ 

	struct MinHeapNode* t = *a; 
	*a = *b; 
	*b = t; 
} 

// The standard minHeapify function. 
void minHeapify(struct MinHeap* minHeap,int idx) 

{ 

	int smallest = idx; 
	unsigned int left = 2 * idx + 1; 
	unsigned int right = 2 * idx + 2; 

	if (left < minHeap->size && minHeap->array[left]-> 
freq < minHeap->array[smallest]->freq) 
		smallest = left; 

	if (right < minHeap->size && minHeap->array[right]-> 
freq < minHeap->array[smallest]->freq) 
		smallest = right; 

	if (smallest != idx) { 
		swapMinHeapNode(&minHeap->array[smallest], 
						&minHeap->array[idx]); 
		minHeapify(minHeap, smallest); 
	} 
} 

// A utility function to check 
// if size of heap is 1 or not 
int isSizeOne(struct MinHeap* minHeap) 
{ 

	return (minHeap->size == 1); 
} 

// A standard function to extract 
// minimum value node from heap 
struct MinHeapNode* extractMin(struct MinHeap* minHeap) 

{ 

	struct MinHeapNode* temp = minHeap->array[0]; 
	minHeap->array[0] 
		= minHeap->array[minHeap->size - 1]; 

	--minHeap->size; 
	minHeapify(minHeap, 0); 

	return temp; 
} 

// A utility function to insert 
// a new node to Min Heap 
void insertMinHeap(struct MinHeap* minHeap, 
				struct MinHeapNode* minHeapNode) 

{ 

	++minHeap->size; 
	int i = minHeap->size - 1; 

	while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) { 

		minHeap->array[i] = minHeap->array[(i - 1) / 2]; 
		i = (i - 1) / 2; 
	} 

	minHeap->array[i] = minHeapNode; 
} 

// A standard function to build min heap 
void buildMinHeap(struct MinHeap* minHeap) 

{ 

	int n = minHeap->size - 1; 
	int i; 

	for (i = (n - 1) / 2; i >= 0; --i) 
		minHeapify(minHeap, i); 
} 

// A utility function to print an array of size n 
void printArr(int arr[], int n) 
{ 
	int i; 
	for (i = 0; i < n; ++i) 
		printf("%d", arr[i]); 

	printf("\n"); 
} 

// Utility function to check if this node is leaf 
int isLeaf(struct MinHeapNode* root) 

{ 

	return !(root->left) && !(root->right); 
} 

// Creates a min heap of capacity 
// equal to size and inserts all character of 
// data[] in min heap. Initially size of 
// min heap is equal to capacity 
struct MinHeap* createAndBuildMinHeap(char data[], int freq[], int size) 

{ 

	struct MinHeap* minHeap = createMinHeap(size); 

	for (int i = 0; i < size; ++i) 
		minHeap->array[i] = newNode(data[i], freq[i]); 

	minHeap->size = size; 
	buildMinHeap(minHeap); 

	return minHeap; 
} 

// The main function that builds Huffman tree 
struct MinHeapNode* buildHuffmanTree(char data[], int freq[], int size, struct MinHeap* minHeap) 

{ 
	struct MinHeapNode *left, *right, *top; 

	// Step 1: Create a min heap of capacity 
	// equal to size. Initially, there are 
	// modes equal to size. 
	minHeap = createAndBuildMinHeap(data, freq, size); 

	// Iterate while size of heap doesn't become 1 
	while (!isSizeOne(minHeap)) { 

		// Step 2: Extract the two minimum 
		// freq items from min heap 
		left = extractMin(minHeap); 
		right = extractMin(minHeap); 

		// Step 3: Create a new internal 
		// node with frequency equal to the 
		// sum of the two nodes frequencies. 
		// Make the two extracted node as 
		// left and right children of this new node. 
		// Add this node to the min heap 
		// '$' is a special value for internal nodes, not used 
		top = newNode('$', left->freq + right->freq); 

		top->left = left; 
		top->right = right; 

		insertMinHeap(minHeap, top); 
	} 

	// Step 4: The remaining node is the 
	// root node and the tree is complete. 
	return extractMin(minHeap); 
} 

// Prints huffman codes from the root of Huffman Tree. 
// It uses arr[] to store codes 
void printCodes(struct MinHeapNode* root, int arr[], int top){ 
	// Assign 0 to left edge and recur
	if (root->left) {
		arr[top] = 0;
		printCodes(root->left, arr, top + 1);
	} 

	// Assign 1 to right edge and recur
	if (root->right) { 

		arr[top] = 1; 
		printCodes(root->right, arr, top + 1); 
	} 

	// If this is a leaf node, then 
	// it contains one of the input 
	// characters, print the character 
	// and its code from arr[]
	if (isLeaf(root)) { 

		printf("%c: ", root->data); 
		printArr(arr, top); 
	} 
}

// Structurally the same as printCodes, we're just building and LUT this time.
void LUTHelper(struct MinHeapNode* root, int arr[], int top, char*** LUT){
	if (root->left) {
		arr[top] = 0;
		// &(*LUT) is a reference to the original pointer. All of these functions
		// are pointer hell.
		LUTHelper(root->left, arr, top + 1, &(*LUT)); 
	} 
	if (root->right) {
		arr[top] = 1; 
		LUTHelper(root->right, arr, top + 1, &(*LUT)); 
	} 
	if (isLeaf(root)) {
		for(int i = 0; i < top; i++){
			char buf[1];
			itoa(arr[i], buf, 10);
			(*LUT)[(unsigned int)root->data][i] = buf[0];
		}
		// IT'S IMPORTANT TO BE TIDY DAMMIT
		(*LUT)[(unsigned int)root->data][top] = '\0';
	} 
}

char** createLUT(struct MinHeapNode* root){
	char** LUT;
	LUT = malloc(sizeof(char*)*256);
	for(int i = 0; i < 256; i++){
		LUT[i] = malloc(sizeof(char)*50);
		strcpy(LUT[i], "N");
	}

	int arr[MAX_TREE_HT], top = 0; 
	LUTHelper(root, arr, top, &LUT);
	return LUT;
}

// The main function that builds a 
// Huffman Tree and print codes by traversing 
// the built Huffman Tree 
void HuffmanCodes(char data[], int freq[], int size) 

{ 
	// Construct Huffman Tree 
	struct MinHeap* mh = NULL;
	struct MinHeapNode* root 
		= buildHuffmanTree(data, freq, size, mh); 

	// Print Huffman codes using 
	// the Huffman tree built above 
	int arr[MAX_TREE_HT], top = 0; 

	printCodes(root, arr, top); 
} 

void calculateSerialSize(struct MinHeapNode* root, int* size, int sizePerNode){
	*size += sizePerNode;
	if(!root){
		return;
	}
	calculateSerialSize(root->left, size, sizePerNode);
	calculateSerialSize(root->right, size, sizePerNode);
}

// All nodes stored as 3 distinct serial ints. ['data','freq','null']
// *data is value of the data for this node.
// *freq is value of... the frequency for this node.
// *null is a marker to signify that there's no node here. (Node is 
//  null if val == 1)
void serializeHelper(struct MinHeapNode* root, int* array, int* index){
	// If current node is NULL, store marker 
    if (root == NULL) 
    {
		array[*index] = 0;
		array[*index+1] = 0;
		array[*index+2] = 1;
		*index += 3;
        return; 
    } 
	
	array[*index] = root->data;
	array[*index+1] = root->freq;
	array[*index+2] = 0;
	*index += 3;
	
    serializeHelper(root->left, array, index); 
    serializeHelper(root->right, array, index); 
}

//Returns the size of the serizlized array returned to array;
void serialize(struct MinHeapNode* root, int* array){
	int index = 0;
	serializeHelper(root, array, &index);
}

// #PointerNightmare
void deserializeHelper(struct MinHeapNode** root, int* array, int size, int* index){
	unsigned char data = array[*index];
	unsigned freq = array[*index+1];
	int null = array[*index+2];

	*index += 3;
	if(null == 1 || *index == size - 1){
		return;
	}

	*root = newNode(data, freq);
	deserializeHelper(&((*root)->left), array, size, index);
	deserializeHelper(&((*root)->right), array, size, index);
}

// assumption of 3 ints per node.
struct MinHeapNode* deserialize(int* array, int size){
	int index = 0;
	struct MinHeapNode* root;
	deserializeHelper(&root, array, size, &index);
	return root;
}
