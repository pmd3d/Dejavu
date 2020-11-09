#pragma once

#include <variant>

namespace jlimdev
{
    using uint = unsigned int;
    using Timestamp = uint;
    using DifficultyRating = uint;

    constexpr DifficultyRating DifficultyRatingEasiest = 0U;
    constexpr DifficultyRating DifficultyRatingMostDifficult = 100U;

    struct NeverReviewed
    {
        DifficultyRating difficultyRating;
    };

    struct PreviouslyIncorrect
    {
        DifficultyRating difficultyRating;
        Timestamp reviewDate;
    };

    struct PreviouslyFirstCorrect
    {
        DifficultyRating difficultyRating;
        Timestamp reviewDate;
    };

    struct PreviouslyCorrect
    {
        DifficultyRating difficultyRating;
        Timestamp reviewDate;
        Timestamp previousCorrectReview;
    };

    using ReviewItem = std::variant<NeverReviewed, PreviouslyIncorrect, PreviouslyFirstCorrect, PreviouslyCorrect>;
}
