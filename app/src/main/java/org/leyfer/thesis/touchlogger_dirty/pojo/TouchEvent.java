package org.leyfer.thesis.touchlogger_dirty.pojo;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;

@JsonInclude(JsonInclude.Include.NON_NULL)
public class TouchEvent extends Event {
    public enum Prefix {
        UP("Up"),
        DOWN("Down"),
        POINTER_DOWN("Pointer down"),
        PONITER_UP("Pointer up"),
        MOVE("Move"),
        UNKNOWN("Unknown");

        private String name;

        Prefix(String name) {
            this.name = name;
        }

        @Override
        public String toString() {
            return name;
        }

        public static Prefix fromString(String name) {
            for (Prefix prefix : Prefix.values()) {
                if (prefix.toString().equals(name)) {
                    return prefix;
                }
            }

            return UNKNOWN;
        }
    }

    @JsonProperty("prefix")
    private String prefix;

    @JsonProperty("pointer_count")
    private Integer pointerCount;

    @JsonProperty("changed_id")
    private Integer changedId;

    @JsonProperty("pointers")
    private List<Pointer> pointers;

    public String getPrefix() {
        return prefix;
    }

    public void setPrefix(String prefix) {
        this.prefix = prefix;
    }

    public Integer getPointerCount() {
        return pointerCount;
    }

    public void setPointerCount(Integer pointerCount) {
        this.pointerCount = pointerCount;
    }

    public Integer getChangedId() {
        return changedId;
    }

    public void setChangedId(Integer changedId) {
        this.changedId = changedId;
    }

    public List<Pointer> getPointers() {
        return pointers;
    }

    public void setPointers(List<Pointer> pointers) {
        this.pointers = pointers;
    }
}
