/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.amazon.alexa.auto.aacs_annotation_processor;

import com.amazon.alexa.auto.aacs_annotation_api.ContextBroadcastReceiver;
import com.amazon.alexa.auto.aacs_annotation_api.ILifecycleObserver;
import com.amazon.alexa.auto.aacs_annotation_api.NaviProviderModule;
import com.amazon.alexa.auto.aacs_annotation_processor.model.IntentFilter;
import com.amazon.alexa.auto.aacs_annotation_processor.model.IntentFilterBindingSet;
import com.amazon.alexa.auto.aacs_annotation_processor.model.IntentFilterClassBinding;
import com.amazon.alexa.auto.aacs_annotation_processor.model.LifecycleObserver;
import com.amazon.alexa.auto.aacs_annotation_processor.model.LifecycleObserverBindingSet;
import com.amazon.alexa.auto.aacs_annotation_processor.model.LifecycleObserverClassBinding;
import com.amazon.alexa.auto.aacs_annotation_processor.model.NaviProvider;
import com.amazon.alexa.auto.aacs_annotation_processor.model.NaviProviderBindingSet;
import com.amazon.alexa.auto.aacs_annotation_processor.model.NaviProviderClassBinding;
import com.amazon.alexa.auto.aacs_annotation_processor.model.PackageBinding;
import com.amazon.alexa.auto.aacs_annotation_processor.utils.Utility;
import com.squareup.javapoet.ArrayTypeName;
import com.squareup.javapoet.ClassName;
import com.squareup.javapoet.CodeBlock;
import com.squareup.javapoet.FieldSpec;
import com.squareup.javapoet.JavaFile;
import com.squareup.javapoet.MethodSpec;
import com.squareup.javapoet.ParameterSpec;
import com.squareup.javapoet.ParameterizedTypeName;
import com.squareup.javapoet.TypeName;
import com.squareup.javapoet.TypeSpec;

import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import javax.annotation.processing.AbstractProcessor;
import javax.annotation.processing.Messager;
import javax.annotation.processing.RoundEnvironment;
import javax.lang.model.SourceVersion;
import javax.lang.model.element.Element;
import javax.lang.model.element.Modifier;
import javax.lang.model.element.PackageElement;
import javax.lang.model.element.TypeElement;
import javax.lang.model.type.TypeMirror;
import javax.tools.Diagnostic;

@SuppressWarnings("unused")
/**
 *  This class AACSAnnotationProcessor is responsible to read the java class annotations and
 *  auto generate the temporary classes for annotated BroadcastReceivers, their intent filters and
 *  permissions.
 */
public class AACSAnnotationProcessor extends AbstractProcessor {
    /**
     * Processes a set of annotation types on type elements originating from the prior round and
     * returns whether or not these annotations are claimed by this processor. If true is returned,
     * the annotations are claimed and subsequent processors will not be asked to process them;
     * if false is returned, the annotations are unclaimed and subsequent processors may be asked to process them.
     * A processor may always return the same boolean value or may vary the result based on chosen criteria.
     *
     * The input set will be empty if the processor supports "*" and the root elements have no annotations.
     * A Processor must gracefully handle an empty set of annotations.
     * @param set the annotation types requested to be processed
     * @param roundEnvironment environment for information about the current and prior round
     * @return whether or not the set of annotations are claimed by this processor
     */
    @Override
    public boolean process(Set<? extends TypeElement> set, RoundEnvironment roundEnvironment) {
        IntentFilterBindingSet intentFilterBindingSet = new IntentFilterBindingSet();

        Set<? extends Element> intentFilterElementsToBind =
                roundEnvironment.getElementsAnnotatedWith(ContextBroadcastReceiver.class);

        intentFilterElementsToBind.forEach(element -> addElementBinding(intentFilterBindingSet, element));
        Set<JavaFile> intentFilterJavaFiles = generateIntentFilterBindingClasses(intentFilterBindingSet);
        Utility.writeFiles(processingEnv, intentFilterJavaFiles);

        LifecycleObserverBindingSet lifecycleObserverBindingSet = new LifecycleObserverBindingSet();

        Set<? extends Element> lifecycleElementsToBind = roundEnvironment.getElementsAnnotatedWith(
                com.amazon.alexa.auto.aacs_annotation_api.LifecycleObserver.class);
        lifecycleElementsToBind.forEach(element -> addElementBinding(lifecycleObserverBindingSet, element));

        Set<JavaFile> lifecycleObserverJavaFiles = generateLifecycleObsBindingClasses(lifecycleObserverBindingSet);
        Utility.writeFiles(processingEnv, lifecycleObserverJavaFiles);

        NaviProviderBindingSet naviProviderBindingSet = new NaviProviderBindingSet();

        Set<? extends Element> naviProviderElementsToBind = roundEnvironment.getElementsAnnotatedWith(
                com.amazon.alexa.auto.aacs_annotation_api.NaviProviderModule.class);
        naviProviderElementsToBind.forEach(element -> addElementBinding(naviProviderBindingSet, element));
        Set<JavaFile> naviProviderJavaFiles = generateNaviProviderBindingClasses(naviProviderBindingSet);
        Utility.writeFiles(processingEnv, naviProviderJavaFiles);
        return true;
    }

    /**
     * If the processor class is annotated with SupportedAnnotationTypes,
     * return an unmodifiable set with the same set of strings as the annotation.
     * If the class is not so annotated, an empty set is returned.
     * @return the names of the annotation types supported by this processor, or an empty set if none
     */
    @Override
    public Set<String> getSupportedAnnotationTypes() {
        return Collections.singleton(ContextBroadcastReceiver.class.getCanonicalName());
    }

    /**
     * If the processor class is annotated with SupportedSourceVersion, return the source version in the annotation.
     * If the class is not so annotated, SourceVersion.RELEASE_6 is returned.
     * @return the latest source version supported by this processor
     */
    @Override
    public SourceVersion getSupportedSourceVersion() {
        return SourceVersion.latestSupported();
    }

    /**
     * Check every class and add in the binding set
     * @param intentFilterBindingSet model class maintaining the classes
     * @param element classes
     */
    private void addElementBinding(IntentFilterBindingSet intentFilterBindingSet, Element element) {
        IntentFilter elementBinding = getIntentFilterBinding(element);
        if (elementBinding != null) {
            TypeElement classElement = (TypeElement) element;
            if ("android.content.BroadcastReceiver".equals(classElement.getSuperclass().toString())) {
                PackageElement packageElement = Utility.getPackage(classElement);
                intentFilterBindingSet.addBinding(packageElement, classElement, elementBinding);
            } else {
                Messager messager = processingEnv.getMessager();
                messager.printMessage(Diagnostic.Kind.ERROR,
                        classElement.getSimpleName() + " must extend android.content.BroadcastReceiver");
            }
        }
    }

    /**
     * Returns intent filter based on the read annotations for all the elements
     * @param element class elements having annotation of type ContextBroadcastReceiver
     * @return object of intent filter with annotated categories, actions and permissions
     */
    private IntentFilter getIntentFilterBinding(Element element) {
        if (!Utility.isFieldAccessible(element)) {
            Messager messager = processingEnv.getMessager();
            messager.printMessage(
                    Diagnostic.Kind.ERROR, "Field not accessible, it cannot be private or static to bind");
            return null;
        }
        ContextBroadcastReceiver annotation = element.getAnnotation(ContextBroadcastReceiver.class);
        String[] categories = annotation.categories();
        String[] actions = annotation.actions();
        String permission = annotation.permission();
        TypeMirror type = element.asType();
        String name = element.getSimpleName().toString();
        return new IntentFilter(type, name, categories, actions, permission);
    }

    /**
     * Provide set of java files object to be generated
     * @param intentFilterBindingSet
     * @return
     */
    private Set<JavaFile> generateIntentFilterBindingClasses(IntentFilterBindingSet intentFilterBindingSet) {
        Set<JavaFile> files = new HashSet<>();

        for (PackageBinding packageBinding : intentFilterBindingSet.getPackageBindings()) {
            String packageName = packageBinding.getPackageName();
            TypeSpec binderClass = generateContextBroadcastReceivers(packageBinding);

            JavaFile javaFile = JavaFile.builder(packageName, binderClass).build();
            files.add(javaFile);
        }
        return files;
    }

    /**
     * This method contains all the business logic for automatically generating
     * ContextBroadcastReceivers class using javapoet library
     * @param packageBinding
     * @return
     */
    private TypeSpec generateContextBroadcastReceivers(PackageBinding packageBinding) {
        TypeName map = ParameterizedTypeName.get(
                ClassName.get(Map.class), ClassName.get(String.class), ArrayTypeName.of(String.class));

        FieldSpec mBroadcastReceiverCategories =
                FieldSpec.builder(map, "mBroadcastReceiverCategories")
                        .addModifiers(Modifier.PRIVATE, Modifier.FINAL, Modifier.STATIC)
                        .initializer("new $T<>()", ClassName.get("java.util", "HashMap"))
                        .build();

        FieldSpec mBroadcastReceiverActions = FieldSpec.builder(map, "mBroadcastReceiverActions")
                                                      .addModifiers(Modifier.PRIVATE, Modifier.FINAL, Modifier.STATIC)
                                                      .initializer("new $T<>()", ClassName.get("java.util", "HashMap"))
                                                      .build();

        FieldSpec mPermission = null;

        CodeBlock.Builder staticBuilder = CodeBlock.builder();
        for (IntentFilterClassBinding cls : packageBinding.getIntentFilterClassBindings()) {
            for (IntentFilter elementBinding : cls.getIntentFilter()) {
                ArrayTypeName stringArray = ArrayTypeName.of(String.class);
                String literal = "{\"" + String.join("\",\"", elementBinding.getCategories()) + "\"}";

                staticBuilder.addStatement("mBroadcastReceiverCategories.put($S, new $T $L)", elementBinding.getType(),
                        stringArray, literal);
                literal = "{\"" + String.join("\",\"", elementBinding.getActions()) + "\"}";
                staticBuilder.addStatement(
                        "mBroadcastReceiverActions.put($S, new $T $L)", elementBinding.getType(), stringArray, literal);

                if (!"".equals(elementBinding.getPermission())) {
                    mPermission = FieldSpec.builder(String.class, "mPermission")
                                          .addModifiers(Modifier.PRIVATE, Modifier.FINAL, Modifier.STATIC)
                                          .initializer("new String($S)", elementBinding.getPermission())
                                          .build();
                }
            }
        }

        TypeSpec.Builder classBuilder = TypeSpec.classBuilder("ContextBroadcastReceivers")
                                                .addModifiers(Modifier.PUBLIC)
                                                .addField(mBroadcastReceiverCategories)
                                                .addField(mBroadcastReceiverActions);
        if (mPermission != null) {
            classBuilder.addField(mPermission);
        }
        classBuilder.addStaticBlock(staticBuilder.build());

        ClassName set = ClassName.get("java.util", "Set");
        TypeName setOfPackages = ParameterizedTypeName.get(set, TypeName.get(String.class));
        MethodSpec.Builder getReceiversBuilder =
                MethodSpec.methodBuilder("getReceiversBuilder").addModifiers(Modifier.PUBLIC, Modifier.STATIC);
        getReceiversBuilder.returns(setOfPackages);
        getReceiversBuilder.addStatement("return mBroadcastReceiverCategories.keySet()");
        classBuilder.addMethod(getReceiversBuilder.build());

        ArrayTypeName stringArray = ArrayTypeName.of(String.class);
        TypeName paramTypeName = TypeName.get(String.class);
        ParameterSpec paramSpec = ParameterSpec.builder(paramTypeName, "className").build();

        MethodSpec.Builder getCategories =
                MethodSpec.methodBuilder("getCategories").addModifiers(Modifier.PUBLIC, Modifier.STATIC);
        getCategories.returns(stringArray);
        getCategories.addParameter(paramSpec);
        getCategories.addStatement("return mBroadcastReceiverCategories.get(className)");
        classBuilder.addMethod(getCategories.build());

        MethodSpec.Builder getActions =
                MethodSpec.methodBuilder("getActions").addModifiers(Modifier.PUBLIC, Modifier.STATIC);
        getActions.returns(stringArray);
        getActions.addParameter(paramSpec);
        getActions.addStatement("return mBroadcastReceiverActions.get(className)");
        classBuilder.addMethod(getActions.build());

        MethodSpec.Builder getPermission =
                MethodSpec.methodBuilder("getPermission").addModifiers(Modifier.PUBLIC, Modifier.STATIC);
        getPermission.returns(ClassName.get(String.class));
        if (mPermission != null) {
            getPermission.addStatement("return mPermission");
        } else {
            getPermission.addStatement("return null");
        }
        classBuilder.addMethod(getPermission.build());

        return classBuilder.build();
    }

    /**
     * Check every class and add in the binding set
     * @param lifecycleObserverBindingSet
     * @param element
     */
    private void addElementBinding(LifecycleObserverBindingSet lifecycleObserverBindingSet, Element element) {
        LifecycleObserver elementBinding = getLifecycleObserverBinding(element);
        if (elementBinding != null) {
            TypeElement classElement = (TypeElement) element;
            Iterator<TypeMirror> itr = (Iterator<TypeMirror>) classElement.getInterfaces().iterator();
            while (itr.hasNext()) {
                String temp = itr.next().toString();
                if (ILifecycleObserver.class.getCanonicalName().equals(temp)) {
                    PackageElement packageElement = Utility.getPackage(classElement);
                    lifecycleObserverBindingSet.addBinding(packageElement, classElement, elementBinding);
                }
            }
        }
    }

    /**
     * Returns LifecycleObserver based on the read annotations for all the elements
     * @param element class elements having annotation of type LifecycleObserver
     * @return object of LifecycleObserver
     */
    private LifecycleObserver getLifecycleObserverBinding(Element element) {
        if (!Utility.isFieldAccessible(element)) {
            Messager messager = processingEnv.getMessager();
            messager.printMessage(
                    Diagnostic.Kind.ERROR, "Field not accessible, it cannot be private or static to bind");
            return null;
        }
        com.amazon.alexa.auto.aacs_annotation_api.LifecycleObserver annotation =
                element.getAnnotation(com.amazon.alexa.auto.aacs_annotation_api.LifecycleObserver.class);
        TypeMirror type = element.asType();
        String name = element.getSimpleName().toString();
        return new LifecycleObserver(type, name);
    }

    /**
     *
     * @param lifecycleObserverBindingSet
     * @return
     */
    private Set<JavaFile> generateLifecycleObsBindingClasses(LifecycleObserverBindingSet lifecycleObserverBindingSet) {
        Set<JavaFile> files = new HashSet<>();
        for (PackageBinding packageBinding : lifecycleObserverBindingSet.getPackageBindings()) {
            String packageName = packageBinding.getPackageName();
            TypeSpec binderClass = generateLifecycleObserver(packageBinding);
            JavaFile javaFile = JavaFile.builder(packageName, binderClass).build();
            files.add(javaFile);
        }
        return files;
    }

    /**
     * This method contains all the business logic for automatically generating
     * LifecycleObserver class using javapoet library
     * @param packageBinding
     * @return
     */
    private TypeSpec generateLifecycleObserver(PackageBinding packageBinding) {
        ClassName hashSet = ClassName.get("java.util", "HashSet");
        FieldSpec mLifecycleObservers = FieldSpec.builder(hashSet, "mLifecycleObservers")
                                                .addModifiers(Modifier.PRIVATE, Modifier.FINAL, Modifier.STATIC)
                                                .initializer("new $T()", hashSet)
                                                .build();

        CodeBlock.Builder staticBuilder = CodeBlock.builder();
        for (LifecycleObserverClassBinding cls : packageBinding.getLifecycleObserverClassBindings()) {
            for (LifecycleObserver elementBinding : cls.getLifecycleObserver()) {
                staticBuilder.addStatement("mLifecycleObservers.add($S)", elementBinding.getType());
            }
        }

        TypeSpec.Builder classBuilder = TypeSpec.classBuilder("ServiceLifecycleObserver")
                                                .addModifiers(Modifier.PUBLIC)
                                                .addField(mLifecycleObservers);
        classBuilder.addStaticBlock(staticBuilder.build());

        ClassName set = ClassName.get("java.util", "Set");
        TypeName setOfPackages = ParameterizedTypeName.get(set, TypeName.get(String.class));
        MethodSpec.Builder getReceiversBuilder =
                MethodSpec.methodBuilder("getLifecycleObservers").addModifiers(Modifier.PUBLIC, Modifier.STATIC);
        getReceiversBuilder.returns(setOfPackages);
        getReceiversBuilder.addStatement("return mLifecycleObservers");
        classBuilder.addMethod(getReceiversBuilder.build());
        return classBuilder.build();
    }

    private void addElementBinding(NaviProviderBindingSet naviProviderBindingSet, Element element) {
        NaviProvider elementBinding = getNaviProviderBinding(element);
        if (elementBinding != null) {
            TypeElement classElement = (TypeElement) element;
            Iterator<TypeMirror> itr = (Iterator<TypeMirror>) classElement.getInterfaces().iterator();
            while (itr.hasNext()) {
                String temp = itr.next().toString();
                if (temp.endsWith("NaviProvider") || temp.endsWith("NaviSearchProvider")) {
                    PackageElement packageElement = Utility.getPackage(classElement);
                    naviProviderBindingSet.addBinding(packageElement, classElement, elementBinding);
                }
            }
        }
    }

    private NaviProvider getNaviProviderBinding(Element element) {
        if (!Utility.isFieldAccessible(element)) {
            Messager messager = processingEnv.getMessager();
            messager.printMessage(
                    Diagnostic.Kind.ERROR, "Field not accessible, it cannot be private or static to bind");
            return null;
        }
        com.amazon.alexa.auto.aacs_annotation_api.NaviProviderModule annotation =
                element.getAnnotation(com.amazon.alexa.auto.aacs_annotation_api.NaviProviderModule.class);
        boolean enabled = annotation.enabled();
        TypeMirror type = element.asType();
        String name = element.getSimpleName().toString();
        return new NaviProvider(type, name, enabled);
    }

    private Set<JavaFile> generateNaviProviderBindingClasses(NaviProviderBindingSet naviProviderBindingSet) {
        Set<JavaFile> files = new HashSet<>();
        for (PackageBinding packageBinding : naviProviderBindingSet.getPackageBindings()) {
            String packageName = packageBinding.getPackageName();
            TypeSpec binderClass = generateNaviProvider(packageBinding);
            JavaFile javaFile = JavaFile.builder(packageName, binderClass).build();
            files.add(javaFile);
        }
        return files;
    }

    private TypeSpec generateNaviProvider(PackageBinding packageBinding) {
        ClassName hashSet = ClassName.get("java.util", "HashSet");
        FieldSpec mNaviProviderModules = FieldSpec.builder(hashSet, "mNaviProviderModules")
                                                 .addModifiers(Modifier.PRIVATE, Modifier.FINAL, Modifier.STATIC)
                                                 .initializer("new $T()", hashSet)
                                                 .build();
        CodeBlock.Builder staticBuilder = CodeBlock.builder();
        for (NaviProviderClassBinding cls : packageBinding.getNaviProviderClassBinding()) {
            for (NaviProvider elementBinding : cls.getNaviProviderModules()) {
                if (elementBinding.isEnabled()) {
                    staticBuilder.addStatement("mNaviProviderModules.add($S)", elementBinding.getType());
                }
            }
        }
        TypeSpec.Builder classBuilder = TypeSpec.classBuilder("NaviProviderModules")
                                                .addModifiers(Modifier.PUBLIC)
                                                .addField(mNaviProviderModules);
        classBuilder.addStaticBlock(staticBuilder.build());

        ClassName set = ClassName.get("java.util", "Set");
        TypeName setOfPackages = ParameterizedTypeName.get(set, TypeName.get(String.class));
        MethodSpec.Builder getNaviProviderModules =
                MethodSpec.methodBuilder("getNaviProviderModules").addModifiers(Modifier.PUBLIC, Modifier.STATIC);
        getNaviProviderModules.returns(setOfPackages);
        getNaviProviderModules.addStatement("return mNaviProviderModules");
        classBuilder.addMethod(getNaviProviderModules.build());
        return classBuilder.build();
    }
}
