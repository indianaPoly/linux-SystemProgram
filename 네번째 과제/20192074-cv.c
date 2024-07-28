#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define THREADS 4 // 스레드 개수 정의
#define N 3000 // 검사할 숫자의 범위 지정

// 전역 변수 사용
int primes[N]; // 소수를 저장할 배열
int pflag[N]; // 숫자의 소수 여부를 저장할 배열
int total = 0; // 찾은 소수의 개수
int finish = 0; // 끝난 스레드 개수

// 뮤텍스 및 조건 변수 선언
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 뮤텍스 초기화
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; // 조건 변수 초기화

// 숫자를 매개변수로 받아 숫자가 소수인지 확인하는 함수
int is_prime(int v) 
{
  int i;
  int bound = floor(sqrt((double)v)) + 1; // 제곱근 까지 확인을 함.

  for (i = 2; i < bound; i++) {
    // 이미 합성수로 판명된 숫자는 건너 뛴다.
    if (!pflag[i]) // pflag[i]가 0이면 합성수 인 것으로 확인
    {
      continue;
    }

    if (v % i == 0) // 숫자 v가 i로 나누어 떨어지면 합성수이므로
    {
      pflag[v] = 0; // 합성수로 표시한다.
      return 0;
    }
  }

  return (v > 1); // 2 이상의 수는 소수라고 판별
}

// 스레드에서 실행될 함수
void *work(void *arg) {
  int start;
  int end;
  int i;

  start = (N / THREADS) * (*(int *)arg); // 각 스레드의 시작 번호 계산
  end = start + N / THREADS; // 각 스레드의 끝 번호 계산

  for (i = start; i < end; i++) {
    if (is_prime(i)) // 숫자 i가 소수인지 확인
    {
      pthread_mutex_lock(&mutex); // 뮤텍스 잠금
      primes[total] = i; // 소수를 primes 배열에 저장
      total++; // 찾은 소수의 개수 증가
      pthread_cond_signal(&cond); // 조건 변수 신호 보내기
      pthread_mutex_unlock(&mutex); // 뮤텍스 잠금 해제
    }
  }

  pthread_mutex_lock(&mutex); // 뮤텍스 잠금
  finish++; // 작업을 완료한 스레드 수 증가
  pthread_cond_signal(&cond); // 조건 변수 신호 보내기
  pthread_mutex_unlock(&mutex); // 뮤텍스 잠금 해제

  return NULL;
}

int main(int argc, char **argv) {
  int i;
  pthread_t tids[THREADS]; // 스레드 ID 배열
  int ids[THREADS]; // 스레드 번호 배열

  for (i = 0; i < N; i++) {
    pflag[i] = 1; // 모든 숫자를 초기에 소수로 표시
  }

  for (i = 0; i < THREADS; i++) {
    ids[i] = i; // 스레드 번호 초기화
    pthread_create(&tids[i], NULL, work, (void *)&ids[i]); // 스레드 생성 및 실행
  }

  pthread_mutex_lock(&mutex); // 뮤텍스 잠금
  while (finish < THREADS) // 모든 스레드가 작업을 완료할 때까지 대기
  {
    pthread_cond_wait(&cond, &mutex); // 조건 변수를 이용해 대기
  }
  pthread_mutex_unlock(&mutex); // 뮤텍스 잠금 해제

  printf("CV: Number of prime numbers between 2 and %d: %d\n", N, total); // 찾은 소수의 개수 출력

  return 0;
}
