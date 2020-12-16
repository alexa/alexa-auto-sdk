package com.amazon.alexa.auto.apis.app;

import androidx.annotation.NonNull;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Optional;

/**
 * Default implementation of {@link AlexaAppScopedComponents} and
 * {@link AlexaAppScopedComponentsActivator}.
 */
public class DefaultScopedComponents implements AlexaAppScopedComponents, AlexaAppScopedComponentsActivator {
    @NonNull
    private final List<AlexaAppScopedComponent> mScopedComponents = new LinkedList<>();

    @Override
    public <T extends AlexaAppScopedComponent> Optional<T> getComponent(@NonNull Class<T> componentClass) {
        return mScopedComponents.stream()
                .filter(scopedComponent -> componentClass.isAssignableFrom(scopedComponent.getClass()))
                .map(componentClass::cast)
                .findFirst();
    }

    @Override
    public <T extends AlexaAppScopedComponent> void activateScope(@NonNull T scopedComponent) {
        mScopedComponents.add(scopedComponent);
    }

    @Override
    public <T extends AlexaAppScopedComponent> void deactivateScope(@NonNull Class<T> scopedComponentClass) {
        Iterator<AlexaAppScopedComponent> scopedComponents = mScopedComponents.iterator();
        while (scopedComponents.hasNext()) {
            AlexaAppScopedComponent component = scopedComponents.next();
            if (scopedComponentClass.isAssignableFrom(component.getClass())) {
                scopedComponents.remove();
                break;
            }
        }
    }
}
