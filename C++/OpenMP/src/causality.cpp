#include <iostream>
#include <fstream>
#include <cmath>
#include "causality.h"
#include "embedding.h"
#include "dimensions.h"
#include "trimming.h"
#include "probabilities.h"

using namespace std;


double* infer_causality_from_manifolds(double* X, double* Y, double* J, double* Z, unsigned int n, unsigned int emb_dim, unsigned int* k_range, unsigned int len_range, double eps/*=0.05*/, double c/*=3.0*/, double bins/*=20.0*/, double** export_dims_p/*=NULL*/, double** export_stdevs_p/*=NULL*/) {
	
	// calculate kNN distances for the whole k-range
    double** X_nn_distances = NULL;
    double** Y_nn_distances = NULL;
    double** J_nn_distances = NULL;
    double** Z_nn_distances = NULL;
    
    #pragma omp parallel sections
    {
        #pragma omp section
        X_nn_distances = knn_distances(X, n, emb_dim, k_range[len_range-1]);
        
        #pragma omp section
        Y_nn_distances = knn_distances(Y, n, emb_dim, k_range[len_range-1]);
        
        #pragma omp section
        J_nn_distances = knn_distances(J, n, emb_dim, k_range[len_range-1]);
        
        #pragma omp section
        Z_nn_distances = knn_distances(Z, n, emb_dim, k_range[len_range-1]);
    }
	
	// set up data export
	double* export_dims = NULL;
	double* export_stdevs = NULL;

	if (export_dims_p != NULL){
		*export_dims_p = new double[len_range*4];
		export_dims = *export_dims_p;
	}
	if (export_stdevs_p != NULL){
		*export_stdevs_p = new double[len_range*4];
		export_stdevs = *export_stdevs_p;
	}
	
	// explore k-range
	unsigned int k;
	unsigned int trimmed_size;
    double** range_probabilities = new double*[len_range];
	
	for (int i=0; i<len_range; i++) {
		k = k_range[i];
		// estimate local dimensions
        double* x_dims = NULL;
        double* y_dims = NULL;
        double* j_dims = NULL;
        double* z_dims = NULL;
        
        #pragma omp parallel sections
        {
            #pragma omp section
            x_dims = local_dims(X_nn_distances, n, k);
            
            #pragma omp section
            y_dims = local_dims(Y_nn_distances, n, k);
            
            #pragma omp section
            j_dims = local_dims(J_nn_distances, n, k);
            
            #pragma omp section
            z_dims = local_dims(Z_nn_distances, n, k);
        }
		
		// trim dimension estimates
		double** trimmed_data = trim_data(x_dims, y_dims, j_dims, z_dims, n, trimmed_size, eps);
		
		// calculate case probabilities
		double eff_sample_size = 2 * k;
		double* expv = NULL;
		double* cov_m = NULL;
		
		fit_gauss(trimmed_data, trimmed_size, eff_sample_size, &expv, &cov_m);
		range_probabilities[i] = get_probabilities(expv, cov_m, c, bins);
		
		// export data
		if (export_dims != NULL) for (int j=0; j<4; j++) export_dims[i*4 + j] = expv[j];
		if (export_stdevs != NULL) for (int j=0; j<4; j++) export_stdevs[i*4 + j] = sqrt(cov_m[j*5]);
		
		// clear trimmed data
		for (int j=0; j<4; j++) delete[] trimmed_data[j];
		delete[] trimmed_data;
		
		// clear dimension estimates
		delete[] x_dims;
		delete[] y_dims;
		delete[] j_dims;
		delete[] z_dims;
	}
	
	// aggregate probabilities
	double* final_probs = new double[5]{0.0, 0.0, 0.0, 0.0, 0.0};
	
	for (int i=0; i<5; i++) {
		for (int j=0; j<len_range; j++) {
			final_probs[i] += range_probabilities[j][i];
		}
		final_probs[i] /= len_range;
	}
	
	// free memory
	for (int i=0; i<len_range; i++) delete[] range_probabilities[i];
	delete[] range_probabilities;
	
	for (int i=0; i<n; i++) {
		delete[] X_nn_distances[i];
		delete[] Y_nn_distances[i];
		delete[] J_nn_distances[i];
		delete[] Z_nn_distances[i];
	}
	
	delete[] X_nn_distances;
	delete[] Y_nn_distances;
	delete[] J_nn_distances;
	delete[] Z_nn_distances;
	
	return final_probs;
}


double* infer_causality(double* x, double* y, unsigned int n, unsigned int emb_dim, unsigned int tau, unsigned int* k_range, unsigned int len_range, double eps/*=0.05*/, double c/*=3.0*/, double bins/*=20.0*/, unsigned int downsample_rate/*=1*/, double** export_dims_p/*=NULL*/, double** export_stdevs_p/*=NULL*/){
	// embed manifolds
	double** manifolds = get_manifolds(x, y, n, emb_dim, tau, downsample_rate);
	double* X = manifolds[0];
	double* Y = manifolds[1];
	double* J = manifolds[2];
	double* Z = manifolds[3];
	
	n -= (emb_dim - 1) * tau;
	if (downsample_rate > 1) n = n / downsample_rate;
	
	// calculate probabilities from manifolds
	double* final_probs = infer_causality_from_manifolds(X, Y, J, Z, n, emb_dim, k_range, len_range, eps, c, bins, export_dims_p, export_stdevs_p);
	
	// free memory
	delete[] X;
	delete[] Y;
	delete[] J;
	delete[] Z;
	delete[] manifolds;
	
	return final_probs;
}

void infer_causality_R(double* probs_return, double* x, double* y, unsigned int* n, unsigned int* emb_dim, unsigned int* tau, unsigned int* k_range, unsigned int* len_range, double* eps/*=0.05*/, double* c/*=3.0*/, double* bins/*=20.0*/, unsigned int* downsample_rate/*=1*/, double* export_dims/*=NULL*/, double* export_stdevs/*=NULL*/){
    double** exported_dims_p = NULL;
    double** exported_stdevs_p = NULL;
    
    bool export_data = export_dims != NULL && export_stdevs != NULL;
    
    if (export_data) {
        double* exported_dims = NULL;
        double* exported_stdevs = NULL;
        
        exported_dims_p = &exported_dims;
        exported_stdevs_p = &exported_stdevs;
    }
    
	double* probs = infer_causality(x, y, *n, *emb_dim, *tau, k_range, *len_range, *eps, *c, *bins, *downsample_rate, exported_dims_p, exported_stdevs_p);
	
    for (int i=0; i<5; i++) probs_return[i] = probs[i];
    
    if (export_data) {
        for (int i=0; i<*len_range * 4; i++) {
            export_dims[i] = exported_dims_p[0][i];
            export_stdevs[i] = exported_stdevs_p[0][i];
        }
        delete[] exported_dims_p[0];
        delete[] exported_stdevs_p[0];
    }
    
	delete[] probs;
}