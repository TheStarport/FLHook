#pragma once

enum class MongoResult
{
    UnknownFailure,
    FindFailure,
    MatchButNoChange,
    Success,
    PerformedSynchronously,
};
