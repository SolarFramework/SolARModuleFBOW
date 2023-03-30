/**
 * @copyright Copyright (c) 2017 B-com http://www.b-com.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SolARFBOWHelper.h"

namespace SolAR {
namespace MODULES {
namespace FBOW {

datastructure::BoWFeature SolARFBOWHelper::fbow2Solar(const fbow::fBow& fbow)
{
    datastructure::BoWFeature bowFeature;
    for (const auto& it : fbow)
        bowFeature[it.first] = it.second.var;
    return bowFeature;
}

datastructure::BoWLevelFeature SolARFBOWHelper::fbow2Solar(const fbow::fBow2& fbow2)
{
    return fbow2;
}

double SolARFBOWHelper::distanceBoW(const datastructure::BoWFeature& bow1, const datastructure::BoWFeature& bow2, ScoringType scoreType)
{
    datastructure::BoWFeature::const_iterator bow1_it = bow1.begin();
    datastructure::BoWFeature::const_iterator bow2_it = bow2.begin();
    datastructure::BoWFeature::const_iterator bow1_end = bow1.end();
    datastructure::BoWFeature::const_iterator bow2_end = bow2.end();

    double score = 0.;
    while(bow1_it != bow1_end && bow2_it != bow2_end)
    {
        const auto& w1 = bow1_it->second;
        const auto& w2 = bow2_it->second;
        if(bow1_it->first == bow2_it->first)
        {
            switch(scoreType) { 
            case ScoringType::L2_NORM:
                score += w1 * w2;
                break;
            case ScoringType::L1_NORM:
                score += fabs(w1 - w2) - fabs(w1) - fabs(w2);
                break;
            case ScoringType::CHI_SQUARE:
                if(w1 + w2 != 0.0)
                    score += w1 * w2 / (w1 + w2);
                break;
            case ScoringType::BHATTACHARYYA:
                score += sqrt(w1 * w2);
                break;
            case ScoringType::DOT_PRODUCT:
                score += w1 * w2;
                break;
            default:
                // unknown scoring type
                break;
            } 
            // move v1 and v2 forward
            ++bow1_it;
            ++bow2_it;
        }
        else if(bow1_it->first < bow2_it->first)
        {
            // move v1 forward
            while(bow1_it!=bow1_end && bow1_it->first < bow2_it->first)
                ++bow1_it;
        }
        else
        {
            // move v2 forward
            while(bow2_it!=bow2_end && bow2_it->first < bow1_it->first)
                ++bow2_it;
        }
    }

    switch(scoreType) {
    case ScoringType::L2_NORM:
        // ||v - w||_{L2} = sqrt( 2 - 2 * Sum(v_i * w_i) )
        //		for all i | v_i != 0 and w_i != 0 )
        // (Nister, 2006)
        if(score >= 1) // rounding errors
            score = 1.0;
        else
            score = 1.0 - sqrt(1.0 - score); // [0..1]
        break;
    case ScoringType::L1_NORM:
        score = -score/2.0;
        break;
    case ScoringType::CHI_SQUARE:
        score = 2. * score; // [0..1]
        break;
    case ScoringType::BHATTACHARYYA:
        break;
    case ScoringType::DOT_PRODUCT:
        break;
    default:
        break;
    }
   
    return score;
}

double SolARFBOWHelper::distanceL1BoW(const datastructure::BoWFeature& v1, const datastructure::BoWFeature& v2){
	datastructure::BoWFeature::const_iterator v1_it, v2_it;
	const datastructure::BoWFeature::const_iterator v1_end = v1.end();
	const datastructure::BoWFeature::const_iterator v2_end = v2.end();
	v1_it = v1.begin();
	v2_it = v2.begin();
	double score = 0;

	while (v1_it != v1_end && v2_it != v2_end)
	{
		const auto&  vi = v1_it->second;
		const auto& wi = v2_it->second;

		if (v1_it->first == v2_it->first)
		{
			score += fabs(vi - wi) - fabs(vi) - fabs(wi);

			// move v1 and v2 forward
			++v1_it;
			++v2_it;
		}
		else if (v1_it->first < v2_it->first)
		{
			// move v1 forward
			v1_it = v1.lower_bound(v2_it->first);
			// v1_it = (first element >= v2_it.id)
		}
		else
		{
			// move v2 forward
			v2_it = v2.lower_bound(v1_it->first);
			// v2_it = (first element >= v1_it.id)
		}
	}
	// ||v - w||_{L1} = 2 + Sum(|v_i - w_i| - |v_i| - |w_i|) 
	//		for all i | v_i != 0 and w_i != 0 
	// (Nister, 2006)
	// scaled_||v - w||_{L1} = 1 - 0.5 * ||v - w||_{L1}
	score = -score / 2.0;
	return score; // [0..1]
}

double SolARFBOWHelper::distanceChiSquareBoW(const datastructure::BoWFeature& v1, const datastructure::BoWFeature& v2) {
	datastructure::BoWFeature::const_iterator v1_it, v2_it;
	const datastructure::BoWFeature::const_iterator v1_end = v1.end();
	const datastructure::BoWFeature::const_iterator v2_end = v2.end();

	v1_it = v1.begin();
	v2_it = v2.begin();

	double score = 0;

	// all the items are taken into account

	while (v1_it != v1_end && v2_it != v2_end)
	{
		const auto& vi = v1_it->second;
		const auto& wi = v2_it->second;

		if (v1_it->first == v2_it->first)
		{
			// (v-w)^2/(v+w) - v - w = -4 vw/(v+w)
			// we move the -4 out
			if (vi + wi != 0.0) score += vi * wi / (vi + wi);

			// move v1 and v2 forward
			++v1_it;
			++v2_it;
		}
		else if (v1_it->first < v2_it->first)
		{
			// move v1 forward
			v1_it = v1.lower_bound(v2_it->first);
		}
		else
		{
			// move v2 forward
			v2_it = v2.lower_bound(v1_it->first);
		}
	}

	// this takes the -4 into account
	score = 2. * score; // [0..1]

	return score;
}

double SolARFBOWHelper::distanceBhattacharyyaBoW(const datastructure::BoWFeature& v1, const datastructure::BoWFeature& v2) {
	datastructure::BoWFeature::const_iterator v1_it, v2_it;
	const datastructure::BoWFeature::const_iterator v1_end = v1.end();
	const datastructure::BoWFeature::const_iterator v2_end = v2.end();

	v1_it = v1.begin();
	v2_it = v2.begin();

	double score = 0;

	while (v1_it != v1_end && v2_it != v2_end)
	{
		const auto& vi = v1_it->second;
		const auto& wi = v2_it->second;

		if (v1_it->first == v2_it->first)
		{
			score += sqrt(vi * wi);

			// move v1 and v2 forward
			++v1_it;
			++v2_it;
		}
		else if (v1_it->first < v2_it->first)
		{
			// move v1 forward
			v1_it = v1.lower_bound(v2_it->first);
			// v1_it = (first element >= v2_it.id)
		}
		else
		{
			// move v2 forward
			v2_it = v2.lower_bound(v1_it->first);
			// v2_it = (first element >= v1_it.id)
		}
	}
	return score; // already scaled
}

double SolARFBOWHelper::distanceDotProductBoW(const datastructure::BoWFeature& v1, const datastructure::BoWFeature& v2) {
	datastructure::BoWFeature::const_iterator v1_it, v2_it;
	const datastructure::BoWFeature::const_iterator v1_end = v1.end();
	const datastructure::BoWFeature::const_iterator v2_end = v2.end();

	v1_it = v1.begin();
	v2_it = v2.begin();

	double score = 0;

	while (v1_it != v1_end && v2_it != v2_end)
	{
		const auto& vi = v1_it->second;
		const auto& wi = v2_it->second;

		if (v1_it->first == v2_it->first)
		{
			score += vi * wi;

			// move v1 and v2 forward
			++v1_it;
			++v2_it;
		}
		else if (v1_it->first < v2_it->first)
		{
			// move v1 forward
			v1_it = v1.lower_bound(v2_it->first);
			// v1_it = (first element >= v2_it.id)
		}
		else
		{
			// move v2 forward
			v2_it = v2.lower_bound(v1_it->first);
			// v2_it = (first element >= v1_it.id)
		}
	}

	return score; // cannot scale
}

double SolARFBOWHelper::distanceKLSBoW(const datastructure::BoWFeature& v1, const datastructure::BoWFeature& v2) {
	datastructure::BoWFeature::const_iterator v1_it, v2_it;
	const datastructure::BoWFeature::const_iterator v1_end = v1.end();
	const datastructure::BoWFeature::const_iterator v2_end = v2.end();

	v1_it = v1.begin();
	v2_it = v2.begin();

	double score = 0;
	const double LOG_EPS = log(DBL_EPSILON);
	// all the items or v are taken into account

	while (v1_it != v1_end && v2_it != v2_end)
	{
		const auto& vi = v1_it->second;
		const auto& wi = v2_it->second;

		if (v1_it->first == v2_it->first)
		{
			if (vi != 0 && wi != 0) score += vi * log(vi / wi);

			// move v1 and v2 forward
			++v1_it;
			++v2_it;
		}
		else if (v1_it->first < v2_it->first)
		{
			// move v1 forward
			score += vi * (log(vi) - LOG_EPS);
			++v1_it;
		}
		else
		{
			// move v2_it forward, do not add any score
			v2_it = v2.lower_bound(v1_it->first);
			// v2_it = (first element >= v1_it.id)
		}
	}

	// sum rest of items of v
	for (; v1_it != v1_end; ++v1_it)
		if (v1_it->second != 0)
			score += v1_it->second * (log(v1_it->second) - LOG_EPS);

	return score; // cannot be scaled
}
}
}
}
