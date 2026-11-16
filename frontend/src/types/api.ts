export interface ApiSuccess<T> {
  success: true
  data: T
  message: string
}

export interface ApiFailure {
  success: false
  error: { code: string; message: string }
}

export type ApiEnvelope<T> = ApiSuccess<T> | ApiFailure

export class ApiError extends Error {
  constructor(message: string, readonly status: number, readonly code = 'API_ERROR') {
    super(message)
    this.name = 'ApiError'
  }
}
