#include "matrixmultiplication.h"




void matrixmul(int A[N][M], int B[M][P], int AB[N][P]) {
#pragma HLS INTERFACE m_axi port = A depth=1024 offset = slave bundle = a_port
#pragma HLS INTERFACE m_axi port = B depth=1024 offset = slave bundle = b_port
#pragma HLS INTERFACE m_axi port = AB depth=1024 offset = slave bundle = ab_port
#pragma HLS INTERFACE s_axilite register port=return
#pragma HLS RealProbe

  #pragma HLS ARRAY_RESHAPE variable=A complete dim=2
  #pragma HLS ARRAY_RESHAPE variable=B complete dim=1
  /* for each row and column of AB */
  row: for(int i = 0; i < N; ++i) {
    col: for(int j = 0; j < P; ++j) {
      #pragma HLS PIPELINE II=1
      /* compute (AB)i,j */
      int ABij = 0;
    product: for(int k = 0; k < M; ++k) {
        ABij += A[i][k] * B[k][j];
      }
      AB[i][j] = ABij;
    }
  }
}
