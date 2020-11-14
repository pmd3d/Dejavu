#pragma once

#include "ReviewItem.h"
#include "ReviewStrategies.h"
#include "StudySession.h"

using i64 = int64_t;

extern "C" void AddPreviouslyCorrect(int difficultyRatingJs, i64 reviewDateJs, i64 previousCorrectReviewJs);

