package hu.info.hunyadi.test;

public record Residence(String country, String city) {
    String getCountry() {
        return country;
    }

    String getCity() {
        return city;
    }
}
