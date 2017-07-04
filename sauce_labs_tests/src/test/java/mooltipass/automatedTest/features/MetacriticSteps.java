package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Metacritic;

public class MetacriticSteps {

	Metacritic metacritic = new Metacritic(WebDriverFactory.get());
	
	@When("I login metacritic with '(.*)'")
	public void login(String username){
		metacritic.goToLogin();
		metacritic.enterEmail(username);
		String password=System.getenv().get("METAPASS");
		metacritic.enterPassword(password);
		metacritic.submit();
		
	}

	@Then("I should be logged in metacritic")
	public void checkLogin(){
		Assert.assertTrue("Expected User to be logged in",metacritic.checkLogin());
	}
	
	@When("I go to metacritic login page")
	public void pressLogin(){
		metacritic.goToLogin();
		Assert.assertTrue("Expected to be at login page", metacritic.checkAtLoginPage());
		
	}
	
	@When("I logout metacritic")
	public void pressLogout(){
		metacritic.logout();
	}

}
