/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  ConstraintSolver.c                                                                                                 *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in ConstraintSolver.h                                                                                  *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ConstraintSolver.h"
#include <yices.h>

                                                    /* VARIABLES */

context_t *logical_context;             // Yices context where the constraints are saved to be solved
model_t *schedule_model;                // Model where to save the solution that yices find when the context is SAT

                                                /* AUXILIAR FUNCTIONS */

                                                    /* FUNCTIONS */
