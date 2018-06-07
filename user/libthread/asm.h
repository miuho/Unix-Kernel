/** @file asm.h
 *  @brief This file defines the assembly atomic functions.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

/**
 * @brief Atomic exchange
 *
 * @param source The source value.
 * @param destination The address of the destination.
 * @return Old destination value.
 */
int xchg(int *destination, int source);

/**
 * @brief Atomic addition
 *
 * @param source The source value.
 * @param destination The address of the destination.
 * @return Old destination value.
 */
int xadd(int *destination, int source);
