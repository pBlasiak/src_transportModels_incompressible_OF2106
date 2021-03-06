/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "HeliumModel.H"
//#include "volFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(HeliumModel, 0);
    defineRunTimeSelectionTable(HeliumModel, dictionary);

	const Enum<HeliumModel::HeliumThermalPropertiesType>
	HeliumModel::HeliumThermalPropertiesTypeNames_
	({
        { HeliumThermalPropertiesType::thermalExpansion, "beta" },
        { HeliumThermalPropertiesType::AGMCoeff, "AGM" },
        { HeliumThermalPropertiesType::entropy, "s" },
        { HeliumThermalPropertiesType::dynamicViscosity, "eta" },
        { HeliumThermalPropertiesType::specificHeatCapacity, "cp" },
        { HeliumThermalPropertiesType::oneByf, "oneByf" },
        { HeliumThermalPropertiesType::density, "rho" }
	 }
	);

	const Foam::dimensionedScalar
	Foam::HeliumModel::Tlambda_("Tlambda", dimTemperature, 2.16795);

	const Foam::dimensionedScalar
	Foam::HeliumModel::TMin_("TMin", dimTemperature, 1.5);
	
	const Foam::dimensionedScalar
	Foam::HeliumModel::TMax_("TMax", dimTemperature, 2.167);
	
	const Foam::label
	Foam::HeliumModel::indexMin_(0);
	
	const Foam::label
	Foam::HeliumModel::indexMax_(667);
	
	const Foam::dimensionedScalar
	Foam::HeliumModel::dT_("dT", dimTemperature, 0.001);

	#include "staticTables.H"
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::HeliumModel::HeliumModel
(
    const word& name,
    const dictionary& HeliumProperties,
    const volVectorField& U,
    const surfaceScalarField& phi
)
:
    name_(name),
    HeliumProperties_(HeliumProperties),
    U_(U),
    phi_(phi),
    TMinField_
    (
        IOobject
        (
            "TMin",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		TMin_
    ),
    TMaxField_
    (
        IOobject
        (
            "TMax",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		TMax_
    ),
    betaHe_
    (
        IOobject
        (
            "betaHe",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		dimensionedScalar("betaHe", dimless/dimTemperature, 0.0)//,
		//"zeroGradient"
    ),

    AGMHe_
    (
        IOobject
        (
            "AGMHe",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		dimensionedScalar("AGM", dimensionSet(-1,1,1,0,0,0,0), 0.0)//,
		//"zeroGradient"
    ),

    sHe_
    (
        IOobject
        (
            "sHe",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		dimensionedScalar("sHe", dimensionSet(0,2,-2,-1,0,0,0), 0.0)//,
		//"zeroGradient"
    ),

    etaHe_
    (
        IOobject
        (
            "etaHe",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		dimensionedScalar("etaHe", dimensionSet(1,-1,-1,0,0,0,0), 0.0)//,
		//"zeroGradient"
    ),

    cpHe_
    (
        IOobject
        (
            "cpHe",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		dimensionedScalar("cpHe", dimensionSet(0,2,-2,-1,0,0,0), 0.0)//,
		//"zeroGradient"
    ),
	
    onebyf_
    (
        IOobject
        (
            "onebyf",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		dimensionedScalar("onebyf", dimensionSet(3,1,-9,-1,0,0,0), 0.0)//,
		//"zeroGradient"
    ),

    rhoHe_
    (
        IOobject
        (
            "rhoHe",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		dimensionedScalar("rhoHe", dimDensity, 0.0)//,
		//"zeroGradient"
    ),

    rhon_
    (
        IOobject
        (
            "rhon",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		dimensionedScalar("rhon", dimDensity, 0.0)//,
		//"zeroGradient"
    ),

    rhos_
    (
        IOobject
        (
            "rhos",
            U.mesh().time().timeName(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U.mesh(),
		dimensionedScalar("rhos", dimDensity, 0.0)//,
		//"zeroGradient"
    ),

    nu_
    (
        IOobject
        (
            name,
            U_.time().timeName(),
            U_.db(),
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        //calcNu()
        U.mesh(),
		dimensionedScalar("nuHe", dimViscosity, 0.0)//,
		//"zeroGradient"
    ),
	HeThermProps_(7),
	HeThermPropsTables_(7)
{
	// Initializing pointers to thermal-flow properties
	HeThermProps_.set(0, &betaHe_);
	HeThermProps_.set(1, &AGMHe_);
	HeThermProps_.set(2, &sHe_);
	HeThermProps_.set(3, &etaHe_);
	HeThermProps_.set(4, &cpHe_);
	HeThermProps_.set(5, &onebyf_);
	HeThermProps_.set(6, &rhoHe_);

	// Initializing pointers to thermal-flow properties tables
	HeThermPropsTables_.set(0, &betaHeTable_);
	HeThermPropsTables_.set(1, &AGMHeTable_);
	HeThermPropsTables_.set(2, &sHeTable_);
	HeThermPropsTables_.set(3, &etaHeTable_);
	HeThermPropsTables_.set(4, &cpHeTable_);
	HeThermPropsTables_.set(5, &onebyfTable_);
	HeThermPropsTables_.set(6, &rhoHeTable_);
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //
// NOTICE:
// This function should be tested if works good.
// It would be better to implement it without forAll loop
// and without additional loop over patches because it slows down
// calculation probably. The better way is to use OpenFOAM class operators
// but I do not know how to implement this so far.
void Foam::HeliumModel::calcHeProp
(
    volScalarField& vsf, 
	const List<scalar>& vsfTable,
	const volScalarField& T,
	const label maxIndex, 
	const dimensionedScalar dt
)
{
	const scalar TMin(TMin_.value());
	const scalar TMax(TMax_.value());
	const scalar dT(dt.value());

	// Solution with iterators
	//forAll(T, celli)
	//{
	//	if (T[celli] < TMin)
	//	{
	//		PtrList<const List<scalar> >::const_iterator iterTable = HeThermPropsTables_.begin();
	//		forAllIters(HeThermProps_, iter)
	//		{
	//			iter()[celli] = iterTable()[indexMin_];
	//			iterTable++;
	//		}
	//	}
	//	else if (T[celli] > TMax)
	//	{
	//		PtrList<const List<scalar> >::const_iterator iterTable = HeThermPropsTables_.begin();
	//		forAllIters(HeThermProps_, iter)
	//		{
	//			iter()[celli] = iterTable()[maxIndex];
	//			iterTable++;
	//		}
	//	}
	//	else
	//	{
	//	    label index = (T[celli] - TMin)/dT;
	//	    if (index == maxIndex)
	//	    {
	//	    	PtrList<const List<scalar> >::const_iterator iterTable = HeThermPropsTables_.begin();
	//	    	forAllIters(HeThermProps_, iter)
	//	    	{
	//	    		iter()[celli] = iterTable()[maxIndex];
	//				iterTable++;
	//	    	}
	//	    }
	//	    else
	//	    {
	//	    	scalar Ti1 = TMin + index*dT;
	//	    	scalar Ti2 = Ti1 + dT;
	//	    	PtrList<const List<scalar> >::const_iterator iterTable = HeThermPropsTables_.begin();
	//	    	forAllIters(HeThermProps_, iter)
	//	    	{
	//	    		scalar a = (iterTable()[index + 1] - iterTable()[index])/(Ti2 - Ti1);
	//	    		scalar b = iterTable()[index] - a*Ti1;
	//	    		iter()[celli] = a*T[celli] + b;
	//				iterTable++;
	//	    	}
	//	    }
    //    }
	//}

	//forAllIters(HeThermProps_, iter)
	//{
	//	iter->correctBoundaryConditions();
	//}

	// Old solution with forAll loops 
	forAll(vsf, celli)
	{
		if (T[celli] < TMin)
		{
			vsf[celli] = vsfTable[indexMin_];
		}
		else if (T[celli] > TMax)
		{
			vsf[celli] = vsfTable[maxIndex];
		}
		else
		{
			label index = (T[celli] - TMin)/dT;
			if (index == maxIndex)
			{
				vsf[celli] = vsfTable[maxIndex];
			}
			else
			{
				scalar Ti1 = TMin + index*dT;
				scalar Ti2 = Ti1 + dT;
				scalar a = (vsfTable[index + 1] - vsfTable[index])/(Ti2 - Ti1);
				scalar b = vsfTable[index] - a*Ti1;
				vsf[celli] = a*T[celli] + b;
			}
		}
	}

	// if we have this line probably we do not need 
	// the second forAll loop over boundaries but it has to be checked
	//vsf.correctBoundaryConditions();

	forAll(vsf.boundaryField(), patchi)
	{
		forAll(vsf.boundaryField()[patchi], facei)
		{
			if (T[facei] < TMin)
			{
				vsf.boundaryFieldRef()[patchi][facei] = vsfTable[indexMin_];
			}
			else if (T[facei] > TMax)
			{
				vsf.boundaryFieldRef()[patchi][facei] = vsfTable[maxIndex];
			}
			else
			{
				label index = (T[facei] - TMin)/dT;
				if (index == maxIndex)
				{
					vsf.boundaryFieldRef()[patchi][facei] = vsfTable[maxIndex];
				}
				else
				{
					scalar Ti1 = TMin + index*dT;
					scalar Ti2 = Ti1 + dT;
					scalar a = (vsfTable[index + 1] - vsfTable[index])/(Ti2 - Ti1);
					scalar b = vsfTable[index] - a*Ti1;
					vsf.boundaryFieldRef()[patchi][facei] = a*T[facei] + b;
				}
			}
		}
	}
}


bool Foam::HeliumModel::read(const dictionary& HeliumProperties)
{
    HeliumProperties_ = HeliumProperties;

    return true;
}


// ************************************************************************* //
