/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC209H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors: Mustafa Quraish, Bianca Schroeder, Karen Reid
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2021 Karen Reid
 */

#include "dectree.h"

/**
 * Load the binary file, filename into a Dataset and return a pointer to 
 * the Dataset. The binary file format is as follows:
 *
 *     -   4 bytes : `N`: Number of images / labels in the file
 *     -   1 byte  : Image 1 label
 *     - NUM_PIXELS bytes : Image 1 data (WIDTHxWIDTH)
 *          ...
 *     -   1 byte  : Image N label
 *     - NUM_PIXELS bytes : Image N data (WIDTHxWIDTH)
 *
 * You can set the `sx` and `sy` values for all the images to WIDTH. 
 * Use the NUM_PIXELS and WIDTH constants defined in dectree.h
 */
Dataset *load_dataset(const char *filename) {
	// Openning file
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror(filename);
		exit(1);
    }

	// Allocating memory for the dataset
    Dataset *ds = malloc(sizeof(Dataset));
	if (ds == NULL) {
		perror("malloc");
		exit(1);
	}

	// Reading the number of items from the dataset
	if (fread(&(ds->num_items), sizeof(int), 1, file) == 0) {
		fprintf(stderr, "Couldn't read number of images in file: %s", filename);
		exit(1);
	}

	// Allocating memory for all the images
	Image *images = malloc(sizeof(Image) * ds->num_items);
	if (images == NULL) {
		perror("malloc");
		exit(1);
	}

	// Allocating memory for all the labels
	unsigned char *labels = malloc(sizeof(unsigned char) * ds->num_items);
	if (labels == NULL) {
		perror("malloc");
		exit(1);
	}

	// Loop to read each image
	for (int i = 0; i < ds->num_items; i++) {
		images[i].sx=WIDTH;
		images[i].sy=WIDTH;

		// Reading label of the image
		if (fread(&labels[i], sizeof(unsigned char), 1, file) == 0) {
			fprintf(stderr, "Couldn't read label for image %d", i);
			exit(1);
		}	

		// Allocating memory for all the pixels of the image
		images[i].data = malloc(sizeof(unsigned char) * NUM_PIXELS);
		if (images[i].data == NULL) {
			perror("malloc");
			exit(1);
		}

		// Reading all the pixels of the image
		if (fread(images[i].data, sizeof(unsigned char), NUM_PIXELS, file) == 0) {
			fprintf(stderr, "Couldn't read data for image %d", i);
			exit(1);
		}
	}

	ds->images = images;
	ds->labels = labels;

	// Closing the file
	if (fclose(file) != 0) {
		perror(filename);
		exit(1);
	}

    return ds;
}

/**
 * Compute and return the Gini impurity of M images at a given pixel
 * The M images to analyze are identified by the indices array. The M
 * elements of the indices array are indices into data.
 * This is the objective function that you will use to identify the best 
 * pixel on which to split the dataset when building the decision tree.
 *
 * Note that the gini_impurity implemented here can evaluate to NAN 
 * (Not A Number) and will return that value. Your implementation of the 
 * decision trees should ensure that a pixel whose gini_impurity evaluates 
 * to NAN is not used to split the data.  (see find_best_split)
 * 
 * DO NOT CHANGE THIS FUNCTION; It is already implemented for you.
 */
double gini_impurity(Dataset *data, int M, int *indices, int pixel) {
    int a_freq[10] = {0}, a_count = 0;
    int b_freq[10] = {0}, b_count = 0;

    for (int i = 0; i < M; i++) {
        int img_idx = indices[i];

        // The pixels are always either 0 or 255, but using < 128 for generality.
        if (data->images[img_idx].data[pixel] < 128) {
            a_freq[data->labels[img_idx]]++;
            a_count++;
        } else {
            b_freq[data->labels[img_idx]]++;
            b_count++;
        }
    }

    double a_gini = 0, b_gini = 0;
    for (int i = 0; i < 10; i++) {
        double a_i = ((double)a_freq[i]) / ((double)a_count);
        double b_i = ((double)b_freq[i]) / ((double)b_count);
        a_gini += a_i * (1 - a_i);
        b_gini += b_i * (1 - b_i);
    }

    // Weighted average of gini impurity of children
    return (a_gini * a_count + b_gini * b_count) / M;
}

/**
 * Given a subset of M images and the array of their corresponding indices, 
 * find and use the last two parameters (label and freq) to store the most
 * frequent label in the set and its frequency.
 *
 * - The most frequent label (between 0 and 9) will be stored in `*label`
 * - The frequency of this label within the subset will be stored in `*freq`
 * 
 * If multiple labels have the same maximal frequency, return the smallest one.
 */
void get_most_frequent(Dataset *data, int M, int *indices, int *label, int *freq) {
    *label = 0;
    *freq = 0;

    int frequencies[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	for (int i = 0; i < M; i++) {
		frequencies[data->labels[indices[i]]]++;
	}

	*freq = frequencies[0];
    for (int i = 1; i < 10; i++) {
        if (frequencies[i] > frequencies[*label]) {
            *label = i;
            *freq = frequencies[i];
        }
    }
}

/**
 * Given a subset of M images as defined by their indices, find and return
 * the best pixel to split the data. The best pixel is the one which
 * has the minimum Gini impurity as computed by `gini_impurity()` and 
 * is not NAN. (See handout for more information)
 * 
 * The return value will be a number between 0-783 (inclusive), representing
 *  the pixel the M images should be split based on.
 * 
 * If multiple pixels have the same minimal Gini impurity, return the smallest.
 */
int find_best_split(Dataset *data, int M, int *indices) {
	float minimum_impurity = INFINITY;
	int pixel = -1;
    for (int i = 0; i < NUM_PIXELS; i++) {
		float temp_impurity = gini_impurity(data, M, indices, i);
		if (temp_impurity < minimum_impurity) {
			minimum_impurity = temp_impurity;
			pixel = i;
		}
	}
	if (pixel == -1){
		fprintf(stderr, "Could not find split. All gini impurities were NAN");
		exit(1);
	}
    return pixel;
}

/**
 * Create the Decision tree. In each recursive call, we consider the subset of the
 * dataset that correspond to the new node. To represent the subset, we pass 
 * an array of indices of these images in the subset of the dataset, along with 
 * its length M. Be careful to allocate this indices array for any recursive 
 * calls made, and free it when you no longer need the array. In this function,
 * you need to:
 *
 *    - Compute ratio of most frequent image in indices, do not split if the
 *      ration is greater than THRESHOLD_RATIO
 *    - Find the best pixel to split on using `find_best_split`
 *    - Split the data based on whether pixel is less than 128, allocate 
 *      arrays of indices of training images and populate them with the 
 *      subset of indices from M that correspond to which side of the split
 *      they are on
 *    - Allocate a new node, set the correct values and return
 *       - If it is a leaf node set `classification`, and both children = NULL.
 *       - Otherwise, set `pixel` and `left`/`right` nodes 
 *         (using build_subtree recursively). 
 */
DTNode *build_subtree(Dataset *data, int M, int *indices) {
    float frequencies[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	// Calculating the frequency of each label
	for (int i = 0; i < M; i++) {
		frequencies[data->labels[indices[i]]]++;
	}

	// Finding the most frequent label
	int frequent = 0;
	float frequency = frequencies[0];
	for (int i = 1; i < 10; i++) {
		if (frequencies[i] > frequency) {
			frequency = frequencies[i];
			frequent = i;
		}
	}

	// Converting to the most frequent ratio
	frequency = frequency / M;
	
	// Allocating memory a node in the tree
	DTNode *node = malloc(sizeof(DTNode));
	if (node == NULL) {
		perror("malloc");
		exit(1);
	}

	// Checking if freqency ratio is greater than the THRESHOLD_RATIO
	if (frequency > THRESHOLD_RATIO) {
		// If greater, setting the node to be a leaf
		node->classification = frequent;
		node->left = NULL;
		node->right = NULL;
	}
	else {
		// Finding the best pixel to split on
		int best_split = find_best_split(data, M, indices);

		// Creating the indices' arrays after the split
		int left_indices[M];
		int right_indices[M];

		// Splitting the image indices into their respective arrays
		// based on the best pixel split
		int left_M = 0, right_M = 0;
		for (int i = 0; i < M; i++) {
			if (data->images[indices[i]].data[best_split] < 128) {
				left_indices[left_M] = indices[i];
				left_M++;
			}
			else {
				right_indices[right_M] = indices[i];
				right_M++;
			}
		}

		// Setting the node values
		node->pixel = best_split;
		node->classification = -1;
		node->left = build_subtree(data, left_M, left_indices);
		node->right = build_subtree(data, right_M, right_indices);
	}

    return node;
}

/**
 * This is the function exposed to the user. All you should do here is set
 * up the `indices` array correctly for the entire dataset and call 
 * `build_subtree()` with the correct parameters.
 */
DTNode *build_dec_tree(Dataset *data) {

	// int *indices = malloc(sizeof(int) * data->num_items);
	// if (indices == NULL) {
	// 	perror("malloc");
	// 	exit(1);
	// }
	int indices[data->num_items];
	for (int i = 0; i < data->num_items; i++) {
		indices[i] = i;
	}

	DTNode *root = build_subtree(data, data->num_items, indices);

	// free(indices);

    return root;
}

/**
 * Given a decision tree and an image to classify, return the predicted label.
 */
int dec_tree_classify(DTNode *root, Image *img) {
	if (root->left == NULL && root->right == NULL) {
		return root->classification;
	}
    else if (img->data[root->pixel] < 128) {
		return dec_tree_classify(root->left, img);
	}
	else {
		return dec_tree_classify(root->right, img);
	}
}

/**
 * This function frees the Decision tree.
 */
void free_dec_tree(DTNode *node) {
	DTNode *left = node->left;
	DTNode *right = node->right;

	free(node);

	if (left != NULL) {
		free_dec_tree(left);
	}
	if (right != NULL) {
		free_dec_tree(right);
	}
}

/**
 * Free all the allocated memory for the dataset
 */
void free_dataset(Dataset *data) {
    free(data->labels);
    for (int i = 0; i < data->num_items; i++) {
        free(data->images[i].data);
    }
    free(data->images);
    free(data);
}
