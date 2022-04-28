/* Copyright 2012-2020 Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "Sprocket/QtTypeConversions.h"
#include <SkyboltSim/SimMath.h>
#include <SkyboltSim/Components/DynamicBodyComponent.h>
#include <SkyboltSim/Spatial/Position.h>
#include <SkyboltCommon/EnumStrings.h>
#include <SkyboltCommon/Range.h>
#include <SkyboltCommon/StringVector.h>

#include <QString>
#include <QVariant>
#include <QVector3D>
#include <nodes/NodeData>

#include <assert.h>
#include <map>
#include <set>
#include <sstream>

class FlowFunction;

class VariantNodeData : public QtNodes::NodeData
{
public:
	VariantNodeData(const QtNodes::NodeDataType& dataType) : dataType(dataType)
	{
	}

	QtNodes::NodeDataType type() const override
	{
		return dataType;
	}


	virtual QVariant toVariant() const
	{
		return QVariant();
	}

	virtual void fromVariant(const QVariant& variant)
	{
		assert(!"Not implemented");
	}

	QtNodes::NodeDataType dataType;
};

template <typename DataTypeT>
struct VariantNodeDataT : public VariantNodeData
{
public:
	typedef typename DataTypeT DataType;
	VariantNodeDataT(const QtNodes::NodeDataType& type, const DataType& data) : VariantNodeData(type), data(data) {}

	virtual QVariant toVariant() const override
	{
		return QVariant(data);
	}

	virtual void fromVariant(const QVariant& variant) override
	{
		data = variant.value<DataType>();
	}

	DataType data;
};

template <class VectorType>
struct VectorNodeDataWithoutVariant : public VariantNodeData
{
public:
	typedef typename VectorType DataType;
	VectorNodeDataWithoutVariant(const QtNodes::NodeDataType& type, const VectorType& data) :
		VariantNodeData(type), data(data) {}

	VectorType data;
	skybolt::IntRangeInclusive addedRange; // !< Indices in new vector that were added
	skybolt::IntRangeInclusive removedRange; //!< Indices in previous vector that were removed
	skybolt::IntRangeInclusive changedRange; // !< Indices in new vector that were changed
};

template <class VectorType, typename ValueType>
struct VectorNodeData : public VectorNodeDataWithoutVariant<VectorType>
{
public:
	VectorNodeData(const QtNodes::NodeDataType& type, const VectorType& data) :
		VectorNodeDataWithoutVariant(type, data) {}

	QVariant toVariant() const override
	{
		skybolt::StringVector strings(data.size());
		for (size_t i = 0; i < strings.size(); ++i)
		{
			std::stringstream ss;
			ss << data[i];
			strings[i] = ss.str();
		}

		return QString::fromStdString(skybolt::toDelimitedString(strings));
	}

	void fromVariant(const QVariant& variant) override
	{
		skybolt::StringVector strings = skybolt::parseStringList(variant.toString().toStdString());
		data.clear();
		data.reserve(strings.size());
		for (const std::string& str : strings)
		{
			std::stringstream ss(str);
			ValueType value;
			ss >> value;
			data.push_back(value);
		}
	}
};

template <class MapType>
struct MapNodeData : public VariantNodeData
{
public:
	typedef typename MapType DataType;
	MapNodeData(const QtNodes::NodeDataType& type, const MapType& data) :
		VariantNodeData(type), data(data) {}

	MapType data;
	std::set<std::string> removedKeys; //!< Keys in previous vector that were removed
	std::set<std::string> addedKeys; // !< Keys in new vector that were added
	std::set<std::string> changedKeys; // !< Keys in new vector that were changed
};

struct FilenameNodeData : public VariantNodeData
{
	typedef std::string DataType;
	FilenameNodeData(const QString& name, const std::string& data) : VariantNodeData({ typeId(), name }), data(data) {}
	static QString typeId() { return "filename"; }
	
	QVariant toVariant() const override
	{
		return QVariant(QString::fromStdString(data));
	}

	void fromVariant(const QVariant& variant) override
	{
		data = variant.value<QString>().toStdString();
	}

	std::string data;
};

struct StringNodeData : public VariantNodeData
{
	typedef std::string DataType;
	StringNodeData(const QString& name, const std::string& data) : VariantNodeData({ typeId(), name }), data(data) {}
	static QString typeId() { return "string"; }

	QVariant toVariant() const override
	{
		return QVariant(QString::fromStdString(data));
	}

	void fromVariant(const QVariant& variant) override
	{
		data = variant.value<QString>().toStdString();
	}

	std::string data;
};

struct DoubleNodeData : public VariantNodeDataT<double>
{
	DoubleNodeData(const QString& name, double data) : VariantNodeDataT({ typeId(), name }, data) {}
	static QString typeId() { return "double"; }
};

struct IntNodeData : public VariantNodeDataT<int>
{
	IntNodeData(const QString& name, int data) : VariantNodeDataT({ typeId(), name }, data) {}
	static QString typeId() { return "int"; }
};

struct BoolNodeData : public VariantNodeDataT<bool>
{
	BoolNodeData(const QString& name, bool data) : VariantNodeDataT({ typeId(), name }, data) {}
	static QString typeId() { return "bool"; }
};

enum AngleUnit
{
	AngleUnitRadians,
	AngleUnitDegrees
};

DEFINE_ENUM_STRINGS(AngleUnit, "Radians", "Degrees")

struct EnumNodeData : public VariantNodeData
{
	EnumNodeData(const QtNodes::NodeDataType& type, const skybolt::StringVector* enumStrings) : VariantNodeData(type), enumStrings(enumStrings) {}

	virtual int getValueAsInt() const = 0;

	const skybolt::StringVector* enumStrings;
};

template <typename EnumType>
struct EnumNodeDataT : public EnumNodeData
{
	typedef EnumType DataType;
	EnumNodeDataT(const QtNodes::NodeDataType& type, EnumType data) : EnumNodeData(type, &skyboltGetEnumStrings<EnumType>()), data(data) {}

	virtual QVariant toVariant() const override
	{
		return QVariant((int)data);
	}

	virtual void fromVariant(const QVariant& variant) override
	{
		data = (EnumType)variant.value<int>();
	}

	int getValueAsInt() const override { return data; }

	EnumType data;
};

struct AngleUnitNodeData : public EnumNodeDataT<AngleUnit>
{
	AngleUnitNodeData(const QString& name, AngleUnit data) : EnumNodeDataT({ typeId(), name }, data) {}
	static QString typeId() { return "angleUnit"; }
};

struct Vector3NodeData : public VariantNodeData
{
public:
	typedef skybolt::sim::Vector3 DataType;
	Vector3NodeData(const QString& name, const skybolt::sim::Vector3& data) : VariantNodeData({ typeId(), name }), data(data) {}
	static QString typeId() { return "vetor3"; }

	QVariant toVariant() const override
	{
		return  toQVector3D(data);
	}

	void fromVariant(const QVariant& variant) override
	{
		data = toVector3(variant.value<QVector3D>());
	}

	skybolt::sim::Vector3 data;
};

struct EntityNodeData : public VariantNodeData
{
public:
	typedef skybolt::sim::Entity* DataType;
	EntityNodeData(const QString& name, skybolt::sim::Entity* data) : VariantNodeData({ typeId(), name }), data(data) {}
	static QString typeId() { return "entity"; }
	skybolt::sim::Entity* data;
};

struct FunctionNodeData : public VariantNodeData
{
public:
	typedef FlowFunction* DataType;
	FunctionNodeData(const QString& name, FlowFunction* data) : VariantNodeData({ typeId(), name }), data(data) {}
	static QString typeId() { return "function"; }
	FlowFunction* data;
};

struct PositionNodeData : public VariantNodeData
{
public:
	typedef skybolt::sim::PositionPtr DataType;
	PositionNodeData(const QString& name, const skybolt::sim::PositionPtr& data) : VariantNodeData({ typeId(), name }), data(data) {}
	static QString typeId() { return "position"; }
	skybolt::sim::PositionPtr data;
};

struct OrientationNodeData : public VariantNodeData
{
public:
	typedef skybolt::sim::OrientationPtr DataType;
	OrientationNodeData(const QString& name, const skybolt::sim::OrientationPtr& data) : VariantNodeData({ typeId(), name }), data(data) {}
	static QString typeId() { return "orientation"; }
	skybolt::sim::OrientationPtr data;
};

struct StringVectorNodeData : public VectorNodeData<skybolt::StringVector, std::string>
{
	StringVectorNodeData(const QString& name, const skybolt::StringVector& data) : VectorNodeData({ typeId(), name }, data) {}
	static QString typeId() { return "stringVector"; }
};

typedef std::vector<double> DoubleVector;
struct DoubleVectorNodeData : public VectorNodeData<DoubleVector, double>
{
	DoubleVectorNodeData(const QString& name, const DoubleVector& data) : VectorNodeData({ typeId(), name }, data) {}
	static QString typeId() { return "doubleVector"; }
};

typedef std::vector<skybolt::sim::PositionPtr> PositionVector;
struct PositionVectorNodeData : public VectorNodeDataWithoutVariant<PositionVector>
{
	PositionVectorNodeData(const QString& name, const PositionVector& data) : VectorNodeDataWithoutVariant({typeId(), name}, data) {}
	static QString typeId() { return "positionVector"; }
};

typedef std::map<std::string, double> DoubleMap;
typedef std::shared_ptr<DoubleVectorNodeData> DoubleVectorNodeDataPtr;
typedef std::map<std::string, DoubleVectorNodeDataPtr> DoubleVectorMap;
struct DoubleVectorMapNodeData : public MapNodeData<DoubleVectorMap>
{
	DoubleVectorMapNodeData(const QString& name, const DoubleVectorMap& data) : MapNodeData({ typeId(), name }, data) {}
	static QString typeId()  { return "doubleVectorMap"; }
};

typedef std::shared_ptr<DoubleVectorMapNodeData> DoubleVectorMapNodeDataPtr;

struct DoubleMapNodeData : public MapNodeData<DoubleMap>
{
	DoubleMapNodeData(const QString& name, const DoubleMap& data) : MapNodeData({ typeId(), name }, data) {}
	static QString typeId()  { return "doubleMap"; }
};

template <typename T>
const T& checkedCast(const QtNodes::NodeData& data)
{
	const T* derived = dynamic_cast<const T*>(&data);
	if (derived)
	{
		return *derived;
	}

	throw std::runtime_error("Input '" + data.type().name.toStdString() + "' has unexpected type '" + data.type().id.toStdString() + "'");
}

template <typename NodeDataType>
const typename NodeDataType::DataType& getDataOrDefault(const QtNodes::NodeData* data)
{
	if (data)
	{
		return checkedCast<NodeDataType>(*data).data;
	}
	static typename NodeDataType::DataType defaultData = typename NodeDataType::DataType();
	return defaultData;
}
