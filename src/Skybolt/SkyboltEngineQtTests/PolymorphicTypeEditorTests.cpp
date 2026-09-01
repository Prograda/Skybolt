/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <catch2/catch.hpp>

#include <SkyboltEngineQt/Widgets/PolymorphicTypeEditor.h>
#include <SkyboltReflect/Reflection.h>
#include <SkyboltWidgets/Property/QtProperty.h>
#include <SkyboltWidgets/Property/QtPropertyReflection.h>

#include <QApplication>
#include <QComboBox>

using namespace skybolt;

// Test base type
struct TestBase
{
	virtual ~TestBase() = default;
};

// Derived types for testing
struct TestDerivedA : TestBase
{
};

struct TestDerivedB : TestBase
{
};

Q_DECLARE_METATYPE(std::shared_ptr<TestBase>)

class QAppFixture
{
public:
	QAppFixture()
	{
		if (!QApplication::instance())
		{
			static int argc = 1;
			static char appName[] = "test";
			static char* argv[] = { appName, nullptr };
			mApp = std::make_unique<QApplication>(argc, argv);
		}
	}
private:
	std::unique_ptr<QApplication> mApp;
};

static auto createFactoryRegistry()
{
	auto registry = std::make_shared<PolymorphicTypeEditor<TestBase>::FactoryRegistryT>();
	registry->emplace("DerivedA", [] { return std::make_shared<TestDerivedA>(); });
	registry->emplace("DerivedB", [] { return std::make_shared<TestDerivedB>(); });
	return registry;
}

static auto createTestEditor(
	QtValue* value,
	const std::shared_ptr<PolymorphicTypeEditor<TestBase>::FactoryRegistryT>& registry,
	refl::TypeRegistry& typeRegistry)
{
	auto valueTranslatorFactories = std::make_shared<ReflValueTranslatorMap>();
	auto editorWidgetFactoryMap = std::make_shared<PropertyEditorWidgetFactoryMap>();

	return std::make_unique<PolymorphicTypeEditor<TestBase>>(
		value,
		registry,
		&typeRegistry,
		valueTranslatorFactories,
		editorWidgetFactoryMap);
}

TEST_CASE_METHOD(QAppFixture, "PolymorphicTypeEditor populates combo box with factory names")
{
	auto value = createQtValue(QVariant::fromValue(std::shared_ptr<TestBase>()));

	refl::TypeRegistry typeRegistry;
	auto editor = createTestEditor(value.get(), createFactoryRegistry(), typeRegistry);

	auto comboBox = editor->findChild<QComboBox*>();
	REQUIRE(comboBox != nullptr);
	REQUIRE(comboBox->count() == 2);
	CHECK(comboBox->itemText(0).toStdString() == "DerivedA");
	CHECK(comboBox->itemText(1).toStdString() == "DerivedB");
}

TEST_CASE_METHOD(QAppFixture, "PolymorphicTypeEditor selecting type creates correct instance")
{
	auto qtValue = createQtValue(QVariant::fromValue(std::shared_ptr<TestBase>()));

	refl::TypeRegistry typeRegistry;
	auto editor = createTestEditor(qtValue.get(), createFactoryRegistry(), typeRegistry);

	auto comboBox = editor->findChild<QComboBox*>();
	REQUIRE(comboBox != nullptr);

	// Simulate selecting "DerivedA"
	comboBox->setCurrentText("DerivedA");

	auto value = qtValue->value().value<std::shared_ptr<TestBase>>();
	REQUIRE(value != nullptr);
	CHECK(dynamic_cast<TestDerivedA*>(value.get()) != nullptr);

	// Simulate selecting "DerivedB"
	comboBox->setCurrentText("DerivedB");

	value = qtValue->value().value<std::shared_ptr<TestBase>>();
	REQUIRE(value != nullptr);
	CHECK(dynamic_cast<TestDerivedB*>(value.get()) != nullptr);
}

TEST_CASE_METHOD(QAppFixture, "PolymorphicTypeEditor updates combo box when property value changes externally")
{
	auto qtValue = createQtValue(QVariant::fromValue(std::shared_ptr<TestBase>()));

	refl::TypeRegistry typeRegistry;
	auto editor = createTestEditor(qtValue.get(), createFactoryRegistry(), typeRegistry);

	auto comboBox = editor->findChild<QComboBox*>();
	REQUIRE(comboBox != nullptr);

	// Set value to DerivedB externally
	qtValue->setValue(QVariant::fromValue(std::shared_ptr<TestBase>(std::make_shared<TestDerivedB>())));

	CHECK(comboBox->currentText().toStdString() == "DerivedB");
}

TEST_CASE_METHOD(QAppFixture, "PolymorphicTypeEditor automatically selects first available type if property is null")
{
	auto value = createQtValue(QVariant::fromValue(std::shared_ptr<TestBase>()));

	refl::TypeRegistry typeRegistry;
	auto editor = createTestEditor(value.get(), createFactoryRegistry(), typeRegistry);

	// Set to a valid value first
	value->setValue(QVariant::fromValue(std::shared_ptr<TestBase>(std::make_shared<TestDerivedA>())));

	// Then set to null
	value->setValue(QVariant::fromValue(std::shared_ptr<TestBase>()));

	// Combo should select the first available type when value is null
	auto comboBox = editor->findChild<QComboBox*>();
	REQUIRE(comboBox != nullptr);
	CHECK(comboBox->currentText().toStdString() == "DerivedA");
}

TEST_CASE_METHOD(QAppFixture, "PolymorphicTypeEditor combobox is empty when factory registry is empty")
{
	auto value = createQtValue(QVariant::fromValue(std::shared_ptr<TestBase>()));

	refl::TypeRegistry typeRegistry;
	auto emptyRegistry = std::make_shared<PolymorphicTypeEditor<TestBase>::FactoryRegistryT>();
	auto editor = createTestEditor(value.get(), emptyRegistry, typeRegistry);

	auto comboBox = editor->findChild<QComboBox*>();
	REQUIRE(comboBox != nullptr);
	CHECK(comboBox->count() == 0);
}
